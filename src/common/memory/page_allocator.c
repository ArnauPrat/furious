


#include "../platform.h"
#include "page_allocator.h"
#include <string.h>

typedef struct fdb_page_alloc_header_t
{
  void* p_next;
} fdb_page_alloc_header_t;

void* 
fdb_page_alloc_alloc_wrapper(void* palloc, 
                             uint32_t alignment, 
                             uint32_t size,
                             uint32_t hint)
{
  fdb_page_alloc_t* tpalloc = (fdb_page_alloc_t*)palloc;
  return fdb_page_alloc_alloc(tpalloc, 
                              alignment, 
                              size, 
                              hint);
}
void
fdb_page_alloc_free_wrapper(void* palloc, 
                            void* ptr)
{
  fdb_page_alloc_t* tpalloc = (fdb_page_alloc_t*)palloc;
  fdb_page_alloc_free(tpalloc, ptr);
}

fdb_mem_stats_t
fdb_page_alloc_stats_wrapper(void* palloc)
{
  fdb_page_alloc_t* tpalloc = (fdb_page_alloc_t*)palloc;
  return fdb_page_alloc_stats(tpalloc);
}

void
fdb_page_alloc_init(fdb_page_alloc_t* palloc, 
                    uint64_t sregion, 
                    uint32_t ssmall, 
                    uint32_t slarge)
{
  memset(palloc, 0, sizeof(fdb_page_alloc_t));
  fdb_mem_allocator_t* gallocator = fdb_get_global_mem_allocator();
  palloc->m_sregion = sregion;
  palloc->m_ssize = ssmall;
  palloc->m_lsize = slarge;
  palloc->p_snextfree = NULL;
  palloc->p_lnextfree = NULL;
  palloc->p_data = mem_alloc(gallocator, 
                             FDB_MIN_ALIGNMENT, 
                             sregion, 
                             FDB_NO_HINT);
  uint32_t soffset =  ssmall - ((uint64_t)palloc->p_data) % ssmall;
  if(soffset == ssmall) soffset = 0;
  palloc->p_stop = (void*)((uint64_t)palloc->p_data + soffset);

  void* lbaseaddr = (void*)(((uint64_t)palloc->p_data + sregion) - slarge);
  uint32_t loffset =  (uint64_t)lbaseaddr % slarge;
  palloc->p_ltop = (void*)((uint64_t)lbaseaddr - loffset);
  palloc->m_super.p_mem_state = palloc;
  palloc->m_super.p_mem_alloc = fdb_page_alloc_alloc_wrapper;
  palloc->m_super.p_mem_free = fdb_page_alloc_free_wrapper;
  palloc->m_super.p_mem_stats = fdb_page_alloc_stats_wrapper;

  palloc->m_stats.m_allocated = sregion;

  fdb_mutex_init(&palloc->m_mutex);
}

void
fdb_page_alloc_release(fdb_page_alloc_t* palloc)
{
  fdb_mutex_release(&palloc->m_mutex);
  fdb_mem_allocator_t* gallocator = fdb_get_global_mem_allocator();
  mem_free(gallocator, palloc->p_data);
  memset(palloc, 0, sizeof(fdb_page_alloc_t));
}

void*
fdb_page_alloc_alloc(fdb_page_alloc_t* palloc, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint)
{
  fdb_mutex_lock(&palloc->m_mutex);
  FDB_ASSERT((size == palloc->m_ssize || 
             size == palloc->m_lsize) &&
             "Page allocator allocation size must be either ssize or lsize");

  void* ret = NULL;
  if(size == palloc->m_ssize)
  {
    if(palloc->p_snextfree != NULL)
    {
      fdb_page_alloc_header_t* phdr = (fdb_page_alloc_header_t*)palloc->p_snextfree;
      ret = palloc->p_snextfree;
      palloc->p_snextfree = phdr->p_next;
    }
    else
    {
      ret = palloc->p_stop;
      palloc->p_stop += palloc->m_ssize; 
      palloc->m_stats.m_used += palloc->m_ssize;
      FDB_ASSERT(ret < palloc->p_ltop && "Small and Large pages overlapping");
    }
    FDB_ASSERT((uint64_t)ret % palloc->m_ssize == 0 && "Small page allocated is not aligned to page boundary");
  }
  else
  {
    if(palloc->p_lnextfree != NULL)
    {
      fdb_page_alloc_header_t* phdr = (fdb_page_alloc_header_t*)palloc->p_lnextfree;
      ret = palloc->p_lnextfree;
      palloc->p_lnextfree = phdr->p_next;
    }
    else
    {
      ret = palloc->p_ltop;
      palloc->p_ltop -= palloc->m_lsize; 
      palloc->m_stats.m_used += palloc->m_lsize;
      FDB_ASSERT(ret > (palloc->p_stop + palloc->m_ssize) &&
                 "Small and Large pages overlapping");
    }
    FDB_ASSERT((uint64_t)ret % palloc->m_lsize == 0 && "Large page allocated is not aligned to page boundary");
  }
  fdb_mutex_unlock(&palloc->m_mutex);
  return ret;
}


void
fdb_page_alloc_free(fdb_page_alloc_t* palloc, 
                    void* ptr)
{
  fdb_mutex_lock(&palloc->m_mutex);
  FDB_ASSERT(((ptr >= palloc->p_data && ptr < palloc->p_stop) ||
             (ptr < (palloc->p_data + palloc->m_sregion) && (ptr >= (palloc->p_ltop + palloc->m_lsize)))) &&
             "Pointer address does not belong to page allocator region");

  if((ptr >= palloc->p_data && ptr < palloc->p_stop)) // Small page
  {
    fdb_page_alloc_header_t* hdr = (fdb_page_alloc_header_t*)ptr;
    hdr->p_next = palloc->p_snextfree;
    palloc->p_snextfree = ptr;
    palloc->m_stats.m_used -= palloc->m_ssize;
  }
  else // Large page
  {
    fdb_page_alloc_header_t* hdr = (fdb_page_alloc_header_t*)ptr;
    hdr->p_next = palloc->p_lnextfree;
    palloc->p_lnextfree = ptr;
    palloc->m_stats.m_used -= palloc->m_lsize;
  }
  fdb_mutex_unlock(&palloc->m_mutex);
}

fdb_mem_stats_t
fdb_page_alloc_stats(fdb_page_alloc_t* palloc)
{
  fdb_mutex_lock(&palloc->m_mutex);
  fdb_mem_stats_t stats = palloc->m_stats;
  fdb_mutex_unlock(&palloc->m_mutex);
  return stats;
}
