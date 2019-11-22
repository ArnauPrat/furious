
#include "memory.h"

#include "numa_alloc.h"

namespace furious
{

furious_alloc_t mem_alloc = numa_alloc;
furious_free_t mem_free = numa_free;

void
__furious_set_malloc(furious_alloc_t alloc_func, 
                     furious_free_t free_func)
{
  mem_alloc = alloc_func;
  mem_free = free_func;
}
  
} /* furious */ 
