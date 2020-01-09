

#include "pool_allocator.h"
#include "../../common/platform.h"

namespace furious
{

#define FURIOUS_POOL_ALLOC_MIN_ALLOC 4096

struct pool_alloc_t;

void
pool_alloc_flush(pool_alloc_t* pool_alloc);

void* 
pool_alloc_alloc(void* state, 
                  uint32_t alignment, 
                  uint32_t size,
                  uint32_t hint);
void
pool_alloc_free(void* state, 
                  void* ptr);
 

struct pool_alloc_header_t
{
  void*   p_next;
};

struct pool_alloc_t
{
  void*                  p_first_chunk; //< Pointer to the first memory chunk
  void*                  p_last_chunk;  //< Pointer to the last memory chunk
  void*                  p_first_free;  //< Pointer to the first block freed
  uint64_t               m_alignment;   //< The alignment of the blocks
  uint64_t               m_block_size;  //< The size of the allocated blocks
  uint64_t               m_page_size;   //< The size of the preallocation chunks
  uint64_t               m_next_free;   //< The next free offset of the last chunk
  uint64_t               m_grow_offset; //< The amount of offset to grow next_free after each allocation
  mem_allocator_t        m_allocator;   //< The parent memory allocator
};

 
/**
 * \brief Creates a pool allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new pool allocator
 */
mem_allocator_t
pool_alloc_create(uint32_t alignment, 
                   uint32_t block_size, 
                   uint32_t page_size,
                   mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")

  mem_allocator_t use_allocator;
  if(allocator != nullptr)
  {
    use_allocator = *allocator; 
  }
  else
  {
    use_allocator = global_mem_allocator;
  }
  pool_alloc_t* palloc = (pool_alloc_t*)mem_alloc(&use_allocator, 1, sizeof(pool_alloc_t), -1);
  palloc->m_allocator = use_allocator;
  palloc->p_first_free = nullptr;
  int32_t min_alignment = alignment > FURIOUS_MIN_ALIGNMENT ? alignment : FURIOUS_MIN_ALIGNMENT;
  palloc->m_alignment = min_alignment;
  palloc->m_block_size = sizeof(pool_alloc_header_t*) > block_size ? sizeof(pool_alloc_header_t) : block_size;
  palloc->m_page_size = page_size;
  palloc->m_next_free = 0;
  palloc->p_first_chunk = nullptr;
  palloc->p_last_chunk = nullptr;
  uint64_t min_growth = sizeof(pool_alloc_header_t) <= palloc->m_block_size ? palloc->m_block_size : sizeof(pool_alloc_header_t);
  //uint64_t extra = sizeof(pool_alloc_header_t) % palloc->m_alignment == 0 ? 0 : 1;
  //palloc->m_grow_offset = ((min_growth / palloc->m_alignment) + extra) * palloc->m_alignment;
  palloc->m_grow_offset = ((min_growth + palloc->m_alignment - 1) / palloc->m_alignment)  * palloc->m_alignment;

  FURIOUS_ASSERT(palloc->m_grow_offset+palloc->m_block_size <= palloc->m_page_size && "Misconfigured pool block allocator. Chunk size is too small");

  mem_allocator_t mem_allocator = {
                                   palloc, 
                                   pool_alloc_alloc, 
                                   pool_alloc_free
                                  };
  return mem_allocator;
}

void
pool_alloc_destroy(mem_allocator_t* mem_allocator)
{
  pool_alloc_t* palloc = (pool_alloc_t*)mem_allocator->p_mem_state;
  pool_alloc_flush(palloc);
  mem_free(&palloc->m_allocator, palloc);
}

/**
 * \brief Flushes all data allocated with the pool alocator
 *
 * \param pool_alloc The allocator to flush
 */
void
pool_alloc_flush(pool_alloc_t* pool_alloc)
{
  void* next = pool_alloc->p_first_chunk;
  while(next != nullptr)
  {
    pool_alloc_header_t* hdr = (pool_alloc_header_t*)next;
    void* to_release = next;
    next = hdr->p_next;
    mem_free(&pool_alloc->m_allocator, to_release);
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
void* pool_alloc_alloc(void* state, 
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint)
{
  pool_alloc_t* palloc = (pool_alloc_t*)state;
#ifdef FURIOUS_ENABLE_ASSERTS
  uint32_t min_alignment = alignment > FURIOUS_MIN_ALIGNMENT ? alignment : FURIOUS_MIN_ALIGNMENT;
#endif
  FURIOUS_ASSERT(palloc->m_alignment == min_alignment && "Requestes alignment mismatches the pool allocator alignment");
  FURIOUS_ASSERT(palloc->m_block_size == size && "Requested size mismatches the pool allocator size");
  void* ret = nullptr;
  if(palloc->p_first_free != nullptr)
  {
    ret = palloc->p_first_free;
    pool_alloc_header_t* hdr = (pool_alloc_header_t*)palloc->p_first_free;
    palloc->p_first_free = hdr->p_next;
  }
  else
  {
    if(palloc->p_first_chunk == nullptr)
    {
      palloc->p_first_chunk = mem_alloc(&palloc->m_allocator, 
                                        palloc->m_alignment, 
                                        palloc->m_page_size, 
                                        hint);
      pool_alloc_header_t* hdr = (pool_alloc_header_t*)palloc->p_first_chunk;
      hdr->p_next = nullptr;
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
      void* new_chunk = mem_alloc(&palloc->m_allocator, 
                                  palloc->m_alignment, 
                                  palloc->m_page_size, 
                                  hint);
      pool_alloc_header_t* hdr = (pool_alloc_header_t*)palloc->p_last_chunk;
      hdr->p_next = new_chunk;
      palloc->p_last_chunk = new_chunk;
      hdr = (pool_alloc_header_t*)palloc->p_last_chunk;
      hdr->p_next = nullptr;
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
void pool_alloc_free(void* state, 
                      void* ptr )
{
  pool_alloc_t* palloc = (pool_alloc_t*)state;
  pool_alloc_header_t* hdr = (pool_alloc_header_t*)ptr;
  hdr->p_next = palloc->p_first_free;
  palloc->p_first_free = ptr;
}


} /* furious */ 
