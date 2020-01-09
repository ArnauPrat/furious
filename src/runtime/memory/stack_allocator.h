


#ifndef _FURIOUS_STACK_ALLOCATOR_H_
#define _FURIOUS_STACK_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "../../common/memory/memory.h"


namespace furious
{

/**
 * \brief Creates a stack allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new stack allocator
 */
mem_allocator_t
stack_alloc_create(mem_allocator_t* allocator = nullptr);

/**
 * \brief Destroys a stack allocator
 *
 * \param stack_alloc The stack allocator to destroy
 */
void
stack_alloc_destroy(mem_allocator_t* allocator);

  
} /* furious */ 
#endif /* ifndef _FURIOUS_STACK_ALLOCATOR_H_ */
