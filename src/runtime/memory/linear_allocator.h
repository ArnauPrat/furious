


#ifndef _FURIOUS_LINEAR_ALLOCATOR_H_
#define _FURIOUS_LINEAR_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "../../common/memory/memory.h"


namespace furious
{


/**
 * \brief Creates a linear allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new linear allocator
 */
mem_allocator_t
linear_alloc_create(mem_allocator_t* allocator = nullptr);

/**
 * \brief Destroys a linear allocator
 *
 * \param linear_alloc The linear allocator to destroy
 */
void
linear_alloc_destroy(mem_allocator_t* allocator);

  
} /* furious */ 
#endif /* ifndef _FURIOUS_LINEAR_ALLOCATOR_H_ */
