
#include "memory.h"
#include "../platform.h"

#include "numa_alloc.h"

namespace furious
{

mem_allocator_t  global_mem_allocator = {nullptr, numa_alloc, numa_free};

void*
mem_alloc(mem_allocator_t* mem_allocator, 
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
mem_free(mem_allocator_t* mem_allocator, 
         void* ptr)
{
  return mem_allocator->p_mem_free(mem_allocator->p_mem_state, 
                                   ptr);
}

void
furious_set_mem_alloc(mem_allocator_t* allocator)
{
  FURIOUS_ASSERT((allocator->p_mem_alloc != nullptr) && (allocator->p_mem_free != nullptr) && "An allocator must provide both alloc and free functions");
  global_mem_allocator = *allocator;
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

} /* furious */ 
