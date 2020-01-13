

#include "stack_allocator.h"
#include "pool_allocator.h"
#include "../../common/platform.h"

namespace furious
{

#define FURIOUS_STACK_ALLOC_MIN_ALLOC 4096

struct stack_alloc_t;

void
stack_alloc_flush(stack_alloc_t* stack_alloc);

void* 
stack_alloc_alloc(void* state, 
                  uint32_t alignment, 
                  uint32_t size,
                  uint32_t hint);
void
stack_alloc_free(void* state, 
                 void* ptr);
 

struct stack_alloc_header_t
{
  void*                   p_data;
  uint32_t                m_next_free;
  stack_alloc_header_t*  p_next_header;
};

struct stack_alloc_t
{
  stack_alloc_header_t*  p_first_chunk;
  stack_alloc_header_t*  p_last_chunk;
  uint32_t               m_page_size;
  mem_allocator_t        m_allocator;
  mem_allocator_t        m_header_allocator;
};

 
/**
 * \brief Creates a stack allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new stack allocator
 */
mem_allocator_t
stack_alloc_create(uint32_t page_size, 
                   mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")

  stack_alloc_t* salloc = (stack_alloc_t*)mem_alloc(&global_mem_allocator, 
                                                    1, 
                                                    sizeof(stack_alloc_t), 
                                                    FURIOUS_NO_HINT);
  if(allocator != nullptr)
  {
    salloc->m_allocator = *allocator; 
  }
  else
  {
    salloc->m_allocator = global_mem_allocator;
  }
  salloc->m_page_size = page_size;
  salloc->m_header_allocator = pool_alloc_create(32, 
                                                 sizeof(stack_alloc_header_t), 
                                                 salloc->m_page_size, 
                                                 &salloc->m_allocator);

  // allocating first chunk
  salloc->p_first_chunk = (stack_alloc_header_t*)mem_alloc(&salloc->m_header_allocator, 
                                                           32, 
                                                           sizeof(stack_alloc_header_t), 
                                                           FURIOUS_NO_HINT);

  salloc->p_first_chunk->m_next_free = 0;
  salloc->p_first_chunk->p_next_header = nullptr;

  salloc->p_first_chunk->p_data = mem_alloc(&salloc->m_allocator, 
                                            64, 
                                            FURIOUS_STACK_ALLOC_MIN_ALLOC, 
                                            FURIOUS_NO_HINT);

  salloc->p_last_chunk = salloc->p_first_chunk;
  mem_allocator_t mem_allocator = {salloc, 
                                   stack_alloc_alloc, 
                                   stack_alloc_free};
  return mem_allocator;
}

void
stack_alloc_destroy(mem_allocator_t* mem_allocator)
{
  stack_alloc_t* salloc = (stack_alloc_t*)mem_allocator->p_mem_state;
  stack_alloc_flush(salloc);
  pool_alloc_destroy(&salloc->m_header_allocator);
  mem_free(&global_mem_allocator, salloc);
}

/**
 * \brief Flushes all data allocated with the stack alocator
 *
 * \param stack_alloc The allocator to flush
 */
void
stack_alloc_flush(stack_alloc_t* stack_alloc)
{
  stack_alloc_header_t* next_chunk = stack_alloc->p_first_chunk;
  while(next_chunk != nullptr)
  {
    stack_alloc_header_t* tmp = next_chunk;
    next_chunk = tmp->p_next_header;
    mem_free(&stack_alloc->m_allocator, tmp->p_data);
    mem_free(&stack_alloc->m_header_allocator, tmp);
  }
  stack_alloc->p_first_chunk = nullptr;
  stack_alloc->p_last_chunk = nullptr;
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
void* stack_alloc_alloc(void* state, 
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint)
{
  stack_alloc_t* salloc = (stack_alloc_t*)state;
  FURIOUS_ASSERT(size <= salloc->m_page_size && "Requested allocation is too large for the given page size");
  int32_t min_alignment = alignment > FURIOUS_MIN_ALIGNMENT ? alignment : FURIOUS_MIN_ALIGNMENT;
  stack_alloc_header_t* last_chunk = salloc->p_last_chunk;

  if(last_chunk->p_data == nullptr)
  {
    return nullptr;
  }

  uint32_t modulo = ((uint64_t)&(((char*)last_chunk->p_data)[last_chunk->m_next_free])) & (min_alignment-1);
  if(modulo != 0)
  {
    last_chunk->m_next_free += min_alignment - modulo;
    if(last_chunk->m_next_free > salloc->m_page_size)
    {
      last_chunk->m_next_free = salloc->m_page_size;
    }
  }

  void* ret = nullptr;
  if(last_chunk->m_next_free + size < salloc->m_page_size)
  {
    ret = &((char*)last_chunk->p_data)[last_chunk->m_next_free];
    last_chunk->m_next_free += size;
  }
  else
  {
    stack_alloc_header_t* new_chunk = (stack_alloc_header_t*)mem_alloc(&salloc->m_header_allocator, 
                                                                       32, 
                                                                       sizeof(stack_alloc_header_t), 
                                                                       FURIOUS_NO_HINT);
    new_chunk->m_next_free = 0;
    new_chunk->p_next_header = nullptr;
    new_chunk->p_data = mem_alloc(&salloc->m_allocator, 
                                  min_alignment, 
                                  salloc->m_page_size, 
                                  FURIOUS_NO_HINT);

    if(new_chunk->p_data == nullptr)
    {
      FURIOUS_ASSERT(false && "Cannot allocate page for stack allocator");
      return nullptr;
    }

    last_chunk->p_next_header = new_chunk;
    salloc->p_last_chunk = new_chunk;

    uint32_t modulo = ((uint64_t)&(((char*)new_chunk->p_data)[new_chunk->m_next_free])) & (min_alignment-1);
    if(modulo != 0)
    {
      new_chunk->m_next_free += min_alignment - modulo;
    }
    FURIOUS_ASSERT(new_chunk->m_next_free + size <= salloc->m_page_size && "Cannot allocate the requeted size with the given alignment");
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
void stack_alloc_free(void* state, 
                           void* ptr )
{
  // Arnau: Intentionally left blank. The stack allocator is freed using the
  // flush method
}


} /* furious */ 
