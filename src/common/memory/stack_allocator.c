

#include "stack_allocator.h"
#include "pool_allocator.h"
#include "../../common/platform.h"

#include <string.h>

#define FDB_STACK_ALLOC_MIN_ALLOC 4096

static void
fdb_stack_alloc_page_addr_decode(fdb_stack_alloc_t* salloc, 
                               void* addr, 
                               char** base, 
                               uint32_t* offset)
{
    *base = (char*)((uint64_t)addr & salloc->m_page_mask);
    *offset = (uint32_t)((uint64_t)addr & ~(salloc->m_page_mask));
}

void* 
fdb_stack_alloc_alloc_wrapper(void* state, 
                      uint32_t alignment, 
                      uint32_t size,
                      uint32_t hint)
{
  fdb_stack_alloc_t* salloc = (fdb_stack_alloc_t*)state;
  return fdb_stack_alloc_alloc(salloc, 
                               alignment, 
                               size, 
                               hint);

}

void
fdb_stack_alloc_free_wrapper(void* state, 
                             void* ptr)
{
  fdb_stack_alloc_t* salloc = (fdb_stack_alloc_t*)state;
  fdb_stack_alloc_free(salloc, ptr);
}

fdb_mem_stats_t
fdb_stack_alloc_stats_wrapper(void* state)
{
  fdb_stack_alloc_t* salloc = (fdb_stack_alloc_t*)state;
  return fdb_stack_alloc_stats(salloc);
}


void
fdb_stack_alloc_flush(fdb_stack_alloc_t* fdb_stack_alloc);

/**
 * \brief inits a stack allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new stack allocator
 */
void
fdb_stack_alloc_init(fdb_stack_alloc_t* salloc, 
                     uint32_t page_size, 
                     fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
                  (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
                 "Provided allocator is ill-formed.")

  memset(salloc, 0, sizeof(fdb_stack_alloc_t));
  if(allocator != NULL)
  {
    salloc->p_allocator = allocator; 
  }
  else
  {
    salloc->p_allocator = fdb_get_global_mem_allocator();
  }
  salloc->m_page_size = page_size;
  uint32_t shift = fdb_os_pow2_bit(page_size)-1;
  salloc->m_page_mask = ~((1 << shift) - 1);
  salloc->p_next_page = NULL;
  salloc->m_next_offset = 0; 
  salloc->p_last_frame = NULL;
  salloc->m_max_frame_offset = sizeof(void*) * (salloc->m_page_size - sizeof(fdb_stack_alloc_fheader_t)) / sizeof(void*);
  salloc->m_super.p_mem_state = salloc; 
  salloc->m_super.p_mem_alloc = fdb_stack_alloc_alloc_wrapper; 
  salloc->m_super.p_mem_free = fdb_stack_alloc_free_wrapper;
  salloc->m_super.p_mem_stats = fdb_stack_alloc_stats_wrapper;
  fdb_mutex_init(&salloc->m_mutex);
}

void
fdb_stack_alloc_release(fdb_stack_alloc_t* salloc)
{
  fdb_stack_alloc_flush(salloc);
  fdb_mutex_release(&salloc->m_mutex);
}

/**
 * \brief Flushes all data allocated with the stack alocator
 *
 * \param fdb_stack_alloc The allocator to flush
 */
void
fdb_stack_alloc_flush(fdb_stack_alloc_t* salloc)
{
  fdb_mutex_lock(&salloc->m_mutex);
  fdb_stack_alloc_fheader_t* fheader = (fdb_stack_alloc_fheader_t*)salloc->p_last_frame;
  if(fheader != NULL)
  {
    if(fheader->m_next_free == 0)
    {
      fheader = fheader->p_previous_page;
    }
    if(fheader != NULL)
    {
      char* frame_data = (char*)fheader + sizeof(fdb_stack_alloc_fheader_t);
      uint32_t next_offset = fheader->m_next_free - sizeof(void*);
      char* next_alloc;
      memcpy(&next_alloc, frame_data + next_offset, sizeof(void*));

      char* page_addr;
      uint32_t page_offset;
      fdb_stack_alloc_page_addr_decode(salloc, 
                                       next_alloc, 
                                       &page_addr, 
                                       &page_offset);

      char* last_page_observed = page_addr;
      while(next_alloc != NULL)
      {
        char* page_addr;
        uint32_t page_offset;
        fdb_stack_alloc_page_addr_decode(salloc, 
                                         next_alloc, 
                                         &page_addr, 
                                         &page_offset);
        if(page_addr != last_page_observed)
        {
          mem_free(salloc->p_allocator, last_page_observed);
          last_page_observed = page_addr;
        }

        next_offset-=sizeof(void*);
        if(next_offset == 0)
        {
          fheader = (fdb_stack_alloc_fheader_t*)fheader->p_previous_page;
          if(fheader != NULL)
          {
            frame_data = (char*)fheader + sizeof(fdb_stack_alloc_fheader_t);
            next_offset = fheader->m_next_free - sizeof(void*);
            memcpy(&next_alloc, frame_data + next_offset, sizeof(void*));
          }
          else
          {
            next_alloc = NULL;
          }
        }
      }
      mem_free(salloc->p_allocator, last_page_observed);
    }
  }

  fheader = salloc->p_last_frame;
  while(fheader != NULL)
  {
    void* tmp = fheader->p_previous_page;
    mem_free(salloc->p_allocator, fheader);
    fheader = tmp;
  }

  salloc->m_next_offset = 0;
  salloc->p_next_page = NULL;
  salloc->p_last_frame = NULL;
  salloc->m_stats.m_allocated = 0;
  salloc->m_stats.m_used = 0;
  salloc->m_stats.m_lost = 0;
  fdb_mutex_unlock(&salloc->m_mutex);
}

/**
 * @brief Allocates a memory block in a numa node
 *
 * @param ptr The state
 * @param alignment The alignment of the memory allocator 
 * @param size The size in bytes of the memory block to allocate
 * @param hint The hint to the allocator. It corresponds to id of the allocated
 * block
 *
 * @return 
 */
void* fdb_stack_alloc_alloc(fdb_stack_alloc_t* salloc, 
                            uint32_t alignment, 
                            uint32_t size,
                            uint32_t hint)
{
  FDB_ASSERT((size <= salloc->m_page_size) && "Requested allocation is too large for the given page size");
  FDB_ASSERT(salloc->m_page_size >= alignment && "Alignment cannot be larger than the page alignment");
  int32_t min_alignment = alignment > FDB_MIN_ALIGNMENT ? alignment : FDB_MIN_ALIGNMENT;

  fdb_mutex_lock(&salloc->m_mutex);

  uint32_t modulo = ((uint64_t)salloc->p_next_page + salloc->m_next_offset) & (min_alignment-1);
  uint32_t slack = 0;
  if(modulo != 0)
  {
    slack = (min_alignment - modulo);
  }

  if(((salloc->m_next_offset + slack + size) >= salloc->m_page_size) || 
     salloc->p_next_page == NULL)
  {
    salloc->p_next_page = mem_alloc(salloc->p_allocator, 
                          salloc->m_page_size, 
                          salloc->m_page_size, 
                          FDB_NO_HINT);
    FDB_ASSERT(((uint64_t)salloc->p_next_page & ~(salloc->m_page_mask)) == 0);
    salloc->m_next_offset = 0;
    salloc->m_stats.m_allocated += salloc->m_page_size;
    salloc->m_stats.m_used += 1; // we use the used field to count for the number of allocated pages for data (not for frames)
    slack = 0; // we do not need to recompute the slack given that page addresses are aligned to page size
  }


  FDB_ASSERT(((salloc->m_next_offset +slack + size) <= salloc->m_page_size) && "This stack allocator cannot serve the requested memory");

  fdb_stack_alloc_fheader_t* last_frame = (fdb_stack_alloc_fheader_t*)salloc->p_last_frame;
  if(last_frame == NULL || last_frame->m_next_free == salloc->m_max_frame_offset)
  {
    void* new_frame = mem_alloc(salloc->p_allocator, 
                                salloc->m_page_size, 
                                salloc->m_page_size, 
                                FDB_NO_HINT);
    fdb_stack_alloc_fheader_t* header = (fdb_stack_alloc_fheader_t*)new_frame;
    header->p_previous_page = last_frame;
    header->m_next_free = 0;
    if(last_frame != NULL)
    {
      last_frame->p_next_page = new_frame;
    }
    salloc->p_last_frame = new_frame;

    salloc->m_stats.m_allocated += salloc->m_page_size;
  }

  void* ret_addr = (char*)salloc->p_next_page + salloc->m_next_offset + slack;
  fdb_stack_alloc_fheader_t* fheader = (fdb_stack_alloc_fheader_t*)salloc->p_last_frame;
  char* frame_data = salloc->p_last_frame + sizeof(fdb_stack_alloc_fheader_t);
  memcpy(frame_data + fheader->m_next_free, &ret_addr, sizeof(void*));
  fheader->m_next_free += sizeof(void*);


  salloc->m_next_offset += slack + size;
  salloc->m_stats.m_used += slack + size;

  FDB_ASSERT(salloc->m_next_offset <= salloc->m_page_size && "Offset cannot exceed page size");

  if(salloc->m_next_offset == salloc->m_page_size)
  {
    salloc->p_next_page = mem_alloc(salloc->p_allocator, 
                                    salloc->m_page_size, 
                                    salloc->m_page_size, 
                                    FDB_NO_HINT);
    FDB_ASSERT(((uint64_t)salloc->p_next_page & ~(salloc->m_page_mask)) == 0);
    salloc->m_next_offset = 0;

    salloc->m_stats.m_allocated+=salloc->m_page_size;
  }

  FDB_ASSERT(((uint64_t)ret_addr & ~(salloc->m_page_mask)) < salloc->m_page_size);
  FDB_ASSERT(((uint64_t)ret_addr % alignment) == 0 && "Wrong alignment of returned address");
  fdb_mutex_unlock(&salloc->m_mutex);
  return ret_addr;
}



/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void fdb_stack_alloc_free(fdb_stack_alloc_t* salloc, 
                          void* ptr )
{
  // Arnau: Intentionally left blank. The stack allocator is freed using the
  // flush method
}

void
fdb_stack_alloc_pop(fdb_stack_alloc_t* salloc, 
                    void* ptr)
{
  fdb_mutex_lock(&salloc->m_mutex);
  fdb_stack_alloc_fheader_t* fheader = salloc->p_last_frame;
  if(fheader->m_next_free == 0)
  {
    void* tmp = fheader;
    fheader = fheader->p_previous_page;
    mem_free(salloc->p_allocator, tmp);
    salloc->p_last_frame = fheader;
    fheader->p_next_page = NULL;
    salloc->m_stats.m_allocated -= salloc->m_page_size;
  }

  FDB_ASSERT(fheader != NULL && "Cannot pop from an empty stack allocator");
  char* frame_data = (char*)fheader + sizeof(fdb_stack_alloc_fheader_t);
  void* pop_addr;
  memcpy(&pop_addr, frame_data + fheader->m_next_free - sizeof(void*), sizeof(void*));
  fheader->m_next_free -= sizeof(void*);
  FDB_ASSERT(pop_addr == ptr && "Invalid pop addres");
  char* page_addr;
  uint32_t page_offset;
  fdb_stack_alloc_page_addr_decode(salloc, pop_addr, &page_addr, &page_offset);
  if(salloc->p_next_page != page_addr)
  {
    mem_free(salloc->p_allocator, salloc->p_next_page);
    salloc->m_stats.m_allocated -= salloc->m_page_size;
    salloc->m_stats.m_used -= 1; // We decremenet one page. We use this field to compute the number of allocated pages (not frames)
  }
  else
  {
    salloc->m_stats.m_used -= salloc->m_next_offset - page_offset;
  }
  salloc->p_next_page = page_addr;
  salloc->m_next_offset = page_offset;
  salloc->m_stats.m_used -= salloc->m_page_size;
  fdb_mutex_unlock(&salloc->m_mutex);
}

fdb_mem_stats_t
fdb_stack_alloc_stats(fdb_stack_alloc_t* salloc)
{
  fdb_mutex_lock(&salloc->m_mutex);
  fdb_mem_stats_t stats = salloc->m_stats;
  stats.m_used = salloc->m_stats.m_used*salloc->m_page_size + salloc->m_next_offset; // We compute the actual used space
  fdb_mutex_unlock(&salloc->m_mutex);
  return stats;
}
