

#include "numa_alloc.h"
#include <numa.h>
#include <memory.h>
#include <stdlib.h>

namespace furious {

int32_t numa_nodes() {
  return 0;
}

void* numa_alloc(void* state, 
                 uint32_t alignment, 
                 uint32_t size,
                 uint32_t hint) 
{

  uint32_t min_alignment = alignment < 16 ? 16 : alignment;
  uint32_t residual = size % min_alignment;
  uint32_t real_size = size;
  if(residual != 0)
  {
     real_size += (min_alignment-residual);
  }
  return aligned_alloc(min_alignment, real_size);
}

void numa_free( void* state, 
                void* ptr ) {
  free(ptr);
}
  
} /* furious */ 
