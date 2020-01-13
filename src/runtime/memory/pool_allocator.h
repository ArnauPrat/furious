


#ifndef _FURIOUS_POOL_ALLOCATOR_H_
#define _FURIOUS_POOL_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "../../common/memory/memory.h"


namespace furious
{

/**
 * \brief Creates a pool allocator
 *
 * \param alignment The alignment of the allocations
 * \param block_size The size of the allocations
 * \param page_size The size of the batches to preallocate
 * \param allocator The parent allocator to use by this allocator
 *
 * \return  Returns the memory allocator
 */
mem_allocator_t
pool_alloc_create(uint32_t alignment, 
                  uint32_t block_size, 
                  uint32_t page_size, 
                  mem_allocator_t* allocator = nullptr);

/**
 * \brief Destroys a pool allocator
 *
 * \param pool_alloc The pool allocator to destroy
 */
void
pool_alloc_destroy(mem_allocator_t* allocator);

  
} /* furious */ 
#endif /* ifndef _FURIOUS_pool_ALLOCATOR_H_ */
