
#ifndef _FURIOUS_ALLOCATOR_H_
#define _FURIOUS_ALLOCATOR_H_

#include "memory.h"

namespace furious
{

struct allocator_t
{
  furious_alloc_t mem_alloc;
  furious_free_t  mem_free;
};

} /*  furious */ 

#endif
