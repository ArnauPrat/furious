

#ifndef _FURIOUS_NUMA_ALLOC_H_
#define _FURIOUS_NUMA_ALLOC_H_ value

#include "../common/types.h"

namespace furious  {

constexpr int32_t ALIGNMENT = 64; 

/**
 * @brief Gets the number of numa nodes available in the system
 *
 * @return Returns the number of numa nodes in the system
 */
int32_t numa_nodes();


/**
 * @brief Allocates a memory block in a numa node
 *
 * @param node The node to allocate the memory block at
 * @param size The size in bytes of the memory block to allocate
 *
 * @return 
 */
void* numa_alloc( int32_t node, int32_t size );


/**
 * @brief Frees an allocated memory block
 *
 * @param ptr The pointer to the memory block to free
 */
void numa_free( void* ptr );


  
} /* furious  */ 

#endif /* ifndef _FURIOUS_NUMA_ALLOC_H_ */
