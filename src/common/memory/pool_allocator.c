

#include "pool_allocator.h"
#include "../../common/platform.h"


#define FDB_POOL_ALLOC_MIN_ALLOC 4096


void* 
fdb_pool_alloc_alloc_wrapper(void* palloc, 
                             uint32_t alignment, 
                             uint32_t size,
                             uint32_t hint)
{
  fdb_pool_alloc_t* tpalloc = (fdb_pool_alloc_t*)palloc;
  return fdb_pool_alloc_alloc(tpalloc, 
                              alignment, 
                              size, 
                              hint);
}

void
fdb_pool_alloc_free_wrapper(void* palloc, 
                            void* ptr)
{
  fdb_pool_alloc_t* tpalloc = (fdb_pool_alloc_t*)palloc;
  fdb_pool_alloc_free(tpalloc, ptr);
}

/**
 * \brief inits a pool allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new pool allocator
 */
void
fdb_pool_alloc_init(fdb_pool_alloc_t* palloc, 
                    uint32_t alignment, 
                    uint32_t block_size, 
                    uint32_t page_size,
                    fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  if(allocator != NULL)
  {
    palloc->p_allocator = allocator; 
  }
  else
  {
    palloc->p_allocator = fdb_get_global_mem_allocator();
  }
  palloc->p_first_free = NULL;
  int32_t min_alignment = alignment > FDB_MIN_ALIGNMENT ? alignment : FDB_MIN_ALIGNMENT;
  palloc->m_alignment = min_alignment;
  palloc->m_block_size = sizeof(fdb_pool_alloc_header_t*) > block_size ? sizeof(fdb_pool_alloc_header_t) : block_size;
  palloc->m_page_size = page_size;
  palloc->m_next_free = 0;
  palloc->p_first_chunk = NULL;
  palloc->p_last_chunk = NULL;
  uint64_t min_growth = sizeof(fdb_pool_alloc_header_t) <= palloc->m_block_size ? 
  palloc->m_block_size : 
  sizeof(fdb_pool_alloc_header_t);

  palloc->m_grow_offset = ((min_growth + palloc->m_alignment - 1) / palloc->m_alignment)  * palloc->m_alignment;

  FDB_ASSERT(palloc->m_grow_offset+palloc->m_block_size <= palloc->m_page_size && "Misconfigured pool block allocator. Chunk size is too small");
  palloc->m_super.p_mem_state = palloc;
  palloc->m_super.p_mem_alloc = fdb_pool_alloc_alloc_wrapper;
  palloc->m_super.p_mem_free = fdb_pool_alloc_free_wrapper;
}

void
fdb_pool_alloc_release(fdb_pool_alloc_t* palloc)
{
  fdb_pool_alloc_flush(palloc);
}

void
fdb_pool_alloc_flush(fdb_pool_alloc_t* fdb_pool_alloc)
{
  void* next = fdb_pool_alloc->p_first_chunk;
  while(next != NULL)
  {
    fdb_pool_alloc_header_t* hdr = (fdb_pool_alloc_header_t*)next;
    void* to_release = next;
    next = hdr->p_next;
    mem_free(fdb_pool_alloc->p_allocator, to_release);
  }
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
void* fdb_pool_alloc_alloc(fdb_pool_alloc_t* palloc, 
                           uint32_t alignment, 
                           uint32_t size,
                           uint32_t hint)
{
#ifdef FDB_ENABLE_ASSERTS
  uint32_t min_alignment = alignment > FDB_MIN_ALIGNMENT ? alignment : FDB_MIN_ALIGNMENT;
#endif
  FDB_ASSERT(palloc->m_alignment == min_alignment && "Requested alignment mismatches the pool allocator alignment");
  FDB_ASSERT(palloc->m_block_size == size && "Requested size mismatches the pool allocator size");
  void* ret = NULL;
  if(palloc->p_first_free != NULL)
  {
    ret = palloc->p_first_free;
    fdb_pool_alloc_header_t* hdr = (fdb_pool_alloc_header_t*)palloc->p_first_free;
    palloc->p_first_free = hdr->p_next;
  }
  else
  {
    if(palloc->p_first_chunk == NULL)
    {
      palloc->p_first_chunk = mem_alloc(palloc->p_allocator, 
                                        palloc->m_alignment, 
                                        palloc->m_page_size, 
                                        hint);
      fdb_pool_alloc_header_t* hdr = (fdb_pool_alloc_header_t*)palloc->p_first_chunk;
      hdr->p_next = NULL;
      palloc->m_next_free = palloc->m_grow_offset;
      palloc->p_last_chunk = palloc->p_first_chunk;
    }

    if(palloc->m_next_free + palloc->m_block_size <= palloc->m_page_size)
    {
      ret = &(((char*)palloc->p_last_chunk)[palloc->m_next_free]);
      palloc->m_next_free += palloc->m_grow_offset; 
    }
    else
    {
      void* new_chunk = mem_alloc(palloc->p_allocator, 
                                  palloc->m_alignment, 
                                  palloc->m_page_size, 
                                  hint);
      fdb_pool_alloc_header_t* hdr = (fdb_pool_alloc_header_t*)palloc->p_last_chunk;
      hdr->p_next = new_chunk;
      palloc->p_last_chunk = new_chunk;
      hdr = (fdb_pool_alloc_header_t*)palloc->p_last_chunk;
      hdr->p_next = NULL;
      palloc->m_next_free = palloc->m_grow_offset;
      ret = &(((char*)palloc->p_last_chunk)[palloc->m_next_free]);
      palloc->m_next_free += palloc->m_grow_offset;
    }
  }
  return ret;
}


/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void fdb_pool_alloc_free(fdb_pool_alloc_t* palloc, 
                         void* ptr )
{
  fdb_pool_alloc_header_t* hdr = (fdb_pool_alloc_header_t*)ptr;
  hdr->p_next = palloc->p_first_free;
  palloc->p_first_free = ptr;
}
