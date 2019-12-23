

#include "linear_allocator.h"
#include "../../common/platform.h"

namespace furious
{

#define FURIOUS_LINEAR_ALLOC_MIN_ALLOC 4096

struct linear_alloc_header_t
{
  void*                   p_data;
  uint32_t                m_next_free;
  uint32_t                m_size;
  linear_alloc_header_t*  p_next_header;
};

struct linear_alloc_t
{
  linear_alloc_header_t*  p_first_chunk;
  linear_alloc_header_t*  p_last_chunk;
  mem_allocator_t         m_allocator;
};

void
linear_alloc_flush(linear_alloc_t* linear_alloc);
void* 
linear_alloc_alloc(void* state, 
                   uint32_t alignment, 
                   uint32_t size,
                   uint32_t hint);
void
linear_alloc_free(void* state, 
                  void* ptr);
  
/**
 * \brief Creates a linear allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new linear allocator
 */
mem_allocator_t
linear_alloc_create(mem_allocator_t* allocator)
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
  linear_alloc_t* lalloc = (linear_alloc_t*)mem_alloc(&use_allocator, 1, sizeof(linear_alloc_t), -1);
  lalloc->m_allocator = use_allocator;

  // allocating first chunk
  lalloc->p_first_chunk = (linear_alloc_header_t*)mem_alloc(&lalloc->m_allocator, 1, sizeof(linear_alloc_header_t), -1);
  lalloc->p_first_chunk->m_next_free = 0;
  lalloc->p_first_chunk->p_next_header = nullptr;
  lalloc->p_first_chunk->p_data = mem_alloc(&lalloc->m_allocator, 64, FURIOUS_LINEAR_ALLOC_MIN_ALLOC, -1);
  lalloc->p_first_chunk->m_size = FURIOUS_LINEAR_ALLOC_MIN_ALLOC;
  lalloc->p_last_chunk = lalloc->p_first_chunk;
  mem_allocator_t mem_allocator = {lalloc, 
                                   linear_alloc_alloc, 
                                   linear_alloc_free
                                  };
  return mem_allocator;
}

void
linear_alloc_destroy(mem_allocator_t* mem_allocator)
{
  linear_alloc_t* lalloc = (linear_alloc_t*)mem_allocator->p_mem_state;
  linear_alloc_flush(lalloc);
  mem_free(&lalloc->m_allocator, lalloc);
}

/**
 * \brief Flushes all data allocated with the linear alocator
 *
 * \param linear_alloc The allocator to flush
 */
void
linear_alloc_flush(linear_alloc_t* linear_alloc)
{
  linear_alloc_header_t* next_chunk = linear_alloc->p_first_chunk;
  while(next_chunk != nullptr)
  {
    linear_alloc_header_t* tmp = next_chunk;
    next_chunk = tmp->p_next_header;
    mem_free(&linear_alloc->m_allocator, tmp->p_data);
    mem_free(&linear_alloc->m_allocator, tmp);
  }
  linear_alloc->p_first_chunk = nullptr;
  linear_alloc->p_last_chunk = nullptr;
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
void* linear_alloc_alloc(void* state, 
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint)
{
  int32_t min_alignment = alignment > 16 ? alignment : 16;
  linear_alloc_t* lalloc = (linear_alloc_t*)state;
  linear_alloc_header_t* last_chunk = lalloc->p_last_chunk;
  uint32_t modulo = ((uint64_t)&(((char*)last_chunk->p_data)[last_chunk->m_next_free])) % min_alignment;
  if(modulo != 0)
  {
    last_chunk->m_next_free += min_alignment - modulo;
    if(last_chunk->m_next_free > last_chunk->m_size)
    {
      last_chunk->m_next_free = last_chunk->m_size;
    }
  }

  void* ret = nullptr;
  if(last_chunk->m_next_free + size < last_chunk->m_size)
  {
    ret = &((char*)last_chunk->p_data)[last_chunk->m_next_free];
    last_chunk->m_next_free += size;
  }
  else
  {
    linear_alloc_header_t* new_chunk = (linear_alloc_header_t*)mem_alloc(&lalloc->m_allocator, 1, sizeof(linear_alloc_header_t), FURIOUS_NO_HINT);
    new_chunk->m_next_free = 0;
    new_chunk->p_next_header = nullptr;
    new_chunk->m_size = size + min_alignment > FURIOUS_LINEAR_ALLOC_MIN_ALLOC ? size + min_alignment : FURIOUS_LINEAR_ALLOC_MIN_ALLOC;
    new_chunk->p_data = mem_alloc(&lalloc->m_allocator, min_alignment, new_chunk->m_size, FURIOUS_NO_HINT);
    last_chunk->p_next_header = new_chunk;
    lalloc->p_last_chunk = new_chunk;

    uint32_t modulo = ((uint64_t)&(((char*)new_chunk->p_data)[new_chunk->m_next_free])) % min_alignment;
    if(modulo != 0)
    {
      new_chunk->m_next_free += min_alignment - modulo;
    }
    ret = &((char*)new_chunk->p_data)[new_chunk->m_next_free];
    new_chunk->m_next_free += size;
  }
  return ret;
}


/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void linear_alloc_free(void* state, 
                           void* ptr )
{
  // Arnau: Intentionally left blank. The linear allocator is freed using the
  // flush method
}


} /* furious */ 
