
#include "memory.h"
#include "../platform.h"

#include "numa_alloc.h"

fdb_mem_allocator_t  global_mem_allocator;

void*
mem_alloc(fdb_mem_allocator_t* mem_allocator, 
          uint32_t alignment, 
          uint32_t size, 
          uint32_t hint)
{
  return mem_allocator->p_mem_alloc(mem_allocator->p_mem_state, 
                                    alignment,
                                    size, 
                                    hint);
}

void 
mem_free(fdb_mem_allocator_t* mem_allocator, 
         void* ptr)
{
  return mem_allocator->p_mem_free(mem_allocator->p_mem_state, 
                                   ptr);
}

fdb_mem_allocator_t*
fdb_get_global_mem_allocator()
{
  if(global_mem_allocator.p_mem_alloc == NULL ||
     global_mem_allocator.p_mem_free == NULL)
  {
    global_mem_allocator.p_mem_alloc = fdb_numa_alloc;
    global_mem_allocator.p_mem_free = fdb_numa_free;
  }
  return &global_mem_allocator;
}

uint32_t 
alignment(uint32_t struct_size)
{
  struct_size--;
  struct_size|=struct_size>>1;
  struct_size|=struct_size>>2;
  struct_size|=struct_size>>4;
  struct_size|=struct_size>>8;
  struct_size|=struct_size>>16;
  struct_size++;
  return struct_size;
}
