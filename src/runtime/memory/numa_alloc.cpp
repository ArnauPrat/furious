

#include "numa_alloc.h"
#include <numa.h>
#include <memory.h>
#include <stdlib.h>

namespace furious {

int32_t numa_nodes() {
  return 0;
}

void* numa_alloc(int32_t alignment, 
                 int32_t size,
                 int32_t hint) {
  int32_t residual = size % alignment;
  int32_t real_size = size + (alignment-residual);
  return aligned_alloc(alignment, real_size);
}

void numa_free( void* ptr ) {
  free(ptr);
}
  
} /* furious */ 
