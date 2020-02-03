

#include "stack_allocator.h"
#include "pool_allocator.h"
#include "../../common/platform.h"


#define FDB_STACK_ALLOC_MIN_ALLOC 4096

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

  if(allocator != NULL)
  {
    salloc->p_allocator = allocator; 
  }
  else
  {
    salloc->p_allocator = fdb_get_global_mem_allocator();
  }
  salloc->m_page_size = page_size;
  fdb_pool_alloc_init(&salloc->m_header_allocator, 
                      32, 
                      sizeof(fdb_stack_alloc_header_t), 
                      salloc->m_page_size, 
                      salloc->p_allocator);

  // allocating first chunk

  salloc->p_first_chunk = (fdb_stack_alloc_header_t*)mem_alloc(&salloc->m_header_allocator.m_super, 
                                                              32, 
                                                              sizeof(fdb_stack_alloc_header_t), 
                                                              FDB_NO_HINT);

  salloc->p_first_chunk->m_next_free = 0;
  salloc->p_first_chunk->p_next_header = NULL;

  salloc->p_first_chunk->p_data = mem_alloc(salloc->p_allocator, 
                                            64, 
                                            FDB_STACK_ALLOC_MIN_ALLOC, 
                                            FDB_NO_HINT);

  salloc->p_last_chunk = salloc->p_first_chunk;
  salloc->m_super.p_mem_state = salloc; 
  salloc->m_super.p_mem_alloc = fdb_stack_alloc_alloc_wrapper; 
  salloc->m_super.p_mem_free = fdb_stack_alloc_free_wrapper;
}

void
fdb_stack_alloc_release(fdb_stack_alloc_t* salloc)
{
  fdb_stack_alloc_flush(salloc);
  fdb_pool_alloc_release(&salloc->m_header_allocator);
}

/**
 * \brief Flushes all data allocated with the stack alocator
 *
 * \param fdb_stack_alloc The allocator to flush
 */
void
fdb_stack_alloc_flush(fdb_stack_alloc_t* salloc)
{

  fdb_stack_alloc_header_t* next_chunk = salloc->p_first_chunk;
  while(next_chunk != NULL)
  {
    fdb_stack_alloc_header_t* tmp = next_chunk;
    next_chunk = tmp->p_next_header;
    mem_free(salloc->p_allocator, tmp->p_data);
    mem_free(&salloc->m_header_allocator.m_super, tmp);
  }
  salloc->p_first_chunk = NULL;
  salloc->p_last_chunk = NULL;
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
  FDB_ASSERT(size <= salloc->m_page_size && "Requested allocation is too large for the given page size");
  int32_t min_alignment = alignment > FDB_MIN_ALIGNMENT ? alignment : FDB_MIN_ALIGNMENT;
  fdb_stack_alloc_header_t* last_chunk = salloc->p_last_chunk;

  if(last_chunk->p_data == NULL)
  {
    return NULL;
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

  void* ret = NULL;
  if(last_chunk->m_next_free + size < salloc->m_page_size)
  {
    ret = &((char*)last_chunk->p_data)[last_chunk->m_next_free];
    last_chunk->m_next_free += size;
  }
  else
  {
    fdb_stack_alloc_header_t* new_chunk = (fdb_stack_alloc_header_t*)mem_alloc(&salloc->m_header_allocator.m_super, 
                                                                               32, 
                                                                               sizeof(fdb_stack_alloc_header_t), 
                                                                               FDB_NO_HINT);
    new_chunk->m_next_free = 0;
    new_chunk->p_next_header = NULL;
    new_chunk->p_data = mem_alloc(salloc->p_allocator, 
                                  min_alignment, 
                                  salloc->m_page_size, 
                                  FDB_NO_HINT);

    if(new_chunk->p_data == NULL)
    {
      FDB_ASSERT(false && "Cannot allocate page for stack allocator");
      return NULL;
    }

    last_chunk->p_next_header = new_chunk;
    salloc->p_last_chunk = new_chunk;

    uint32_t modulo = ((uint64_t)&(((char*)new_chunk->p_data)[new_chunk->m_next_free])) & (min_alignment-1);
    if(modulo != 0)
    {
      new_chunk->m_next_free += min_alignment - modulo;
    }
    FDB_ASSERT(new_chunk->m_next_free + size <= salloc->m_page_size && "Cannot allocate the requeted size with the given alignment");
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
void fdb_stack_alloc_free(fdb_stack_alloc_t* salloc, 
                          void* ptr )
{
  // Arnau: Intentionally left blank. The stack allocator is freed using the
  // flush method
}


