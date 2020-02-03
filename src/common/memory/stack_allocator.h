


#ifndef _FURIOUS_STACK_ALLOCATOR_H_
#define _FURIOUS_STACK_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "memory.h"
#include "pool_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_stack_alloc_header_t
{
  void*                             p_data;
  uint32_t                          m_next_free;
  struct fdb_stack_alloc_header_t*  p_next_header;
} fdb_stack_alloc_header_t;

typedef struct fdb_stack_alloc_t
{
  fdb_stack_alloc_header_t*  p_first_chunk;
  fdb_stack_alloc_header_t*  p_last_chunk;
  uint32_t                   m_page_size;
  fdb_mem_allocator_t*       p_allocator;
  fdb_pool_alloc_t           m_header_allocator;
  fdb_mem_allocator_t        m_super;
} fdb_stack_alloc_t;

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
                     fdb_mem_allocator_t* allocator);

/**
 * \brief releases a stack allocator
 *
 * \param fdb_stack_alloc The stack allocator to release
 */
void
fdb_stack_alloc_release(fdb_stack_alloc_t* allocator);

void* 
fdb_stack_alloc_alloc(fdb_stack_alloc_t* state, 
                      uint32_t alignment, 
                      uint32_t size,
                      uint32_t hint);
void
fdb_stack_alloc_free(fdb_stack_alloc_t* state, 
                     void* ptr);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_STACK_ALLOCATOR_H_ */
