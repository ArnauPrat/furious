

#ifndef _FDB_NUMA_ALLOC_H_
#define _FDB_NUMA_ALLOC_H_ value

#include "../../common/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Gets the number of numa nodes available in the system
 *
 * @return Returns the number of numa nodes in the system
 */
int32_t fdb_numa_nodes();


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
void* fdb_numa_alloc(void* state, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint);


/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void fdb_numa_free( void* state, 
                    void* ptr );

#ifdef __cplusplus
}
#endif


#endif /* ifndef _FDB_NUMA_ALLOC_H_ */
