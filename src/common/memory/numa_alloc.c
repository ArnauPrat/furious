
// Required to have posix_memalign enabled
#define _POSIX_C_SOURCE 200809L

#include "../platform.h"
#include "memory.h"
#include "numa_alloc.h"

#include <numa.h>
#include <stdlib.h>


int32_t numa_nodes() {
  return 0;
}

void* fdb_numa_alloc(void* state, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint) 
{

  uint32_t min_alignment = alignment < FDB_MIN_ALIGNMENT ? FDB_MIN_ALIGNMENT : alignment;
  //uint32_t residual = size % min_alignment;
  //uint32_t real_size = size;
  //if(residual != 0)
  //{
  //  real_size += (min_alignment-residual);
  //}
  void* ptr = NULL;
  posix_memalign(&ptr, min_alignment, size);
  return ptr;
}

void fdb_numa_free( void* state, 
                    void* ptr ) {
  free(ptr);
}
