

#include "../../common/types.h"

namespace furious
{

constexpr int32_t ALIGNMENT = 64; 
  
using furious_alloc_t = void* (*) (int32_t,   // alignment 
                                   int32_t,   // size in bytes
                                   int32_t);  // hint 

using furious_free_t = void (*) (void* ptr);


extern furious_alloc_t mem_alloc;
extern furious_free_t mem_free;

/**
 * \brief Sets the memory allocator for furious
 *
 * \param alloc_func
 * \param free_func
 */
void
__furious_set_malloc(furious_alloc_t alloc_func, 
                     furious_free_t free_func);

} /* furious */ 
