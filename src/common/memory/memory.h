
#ifndef _FDB_MEMORY_H_
#define _FDB_MEMORY_H_

#include "../../common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_NO_HINT 0xffffffff
#define BYTES(value) (value)
#define KILOBYTES(value) (BYTES(value)*(uint64_t)1024)
#define MEGABYTES(value) (KILOBYTES(value)*(uint64_t)1024)
#define GIGABYTES(value) (MEGABYTES(value)*(uint64_t)1024)

//constexpr int32_t ALIGNMENT = 64; 
  
typedef void* (*fdb_alloc_t) (void*,      // ptr to state
                              uint32_t,   // alignment 
                              uint32_t,   // size in bytes
                              uint32_t);  // hint with the block id of this allocation. -1 indicates no hint

typedef void (*fdb_free_t) (void*,       // ptr to state
                            void*);      // ptr to free

typedef struct fdb_mem_stats_t
{
  uint64_t m_allocated;
  uint64_t m_lost;
  uint64_t m_used;
} fdb_mem_stats_t;

typedef fdb_mem_stats_t (*fdb_stats_t) (void*);    // ptr to state

typedef struct fdb_mem_allocator_t
{
  void*           p_mem_state;
  fdb_alloc_t     p_mem_alloc;
  fdb_free_t      p_mem_free;
  fdb_stats_t     p_mem_stats;
} fdb_mem_allocator_t;

void* 
mem_alloc(fdb_mem_allocator_t* mem_allocator, 
          uint32_t alignment, 
          uint32_t size, 
          uint32_t hint);

void 
mem_free(fdb_mem_allocator_t* mem_allocator, 
         void* ptr);

fdb_mem_stats_t 
mem_stats(fdb_mem_allocator_t* mem_allocator);

/**
 * \brief gets the global mem  allocator
 *
 * \return A pointer to the global mem allocator
 */
fdb_mem_allocator_t*
fdb_get_global_mem_allocator();

/**
 * \brief Gets the alignment for a structure of the given size 
 *
 * \param struct_size The size in bytes of the structure to get the alignment
 * for
 *
 * \return Returns the alignment
 */
uint32_t 
alignment(uint32_t struct_size);

#ifdef __cplusplus
}
#endif

#endif
