

#include "numa_alloc.h"
#include <numa.h>
#include <memory.h>
#include <stdlib.h>

namespace furious {

int32_t numa_nodes() {
  return 0;
}

void* numa_alloc( int32_t node, int32_t size ) {
  int32_t residual = size % ALIGNMENT;
  int32_t real_size = size + (ALIGNMENT-residual);
  return aligned_alloc(ALIGNMENT, real_size);
}

void numa_free( void* ptr ) {
  free(ptr);
}
  
} /* furious */ 
