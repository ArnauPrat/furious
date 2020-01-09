
#ifndef _FURIOUS_MEMORY_H_
#define _FURIOUS_MEMORY_H_

#include "../../common/types.h"

namespace furious
{

#define FURIOUS_NO_HINT 0xffffffff
#define BYTES(value) (value)
#define KILOBYTES(value) (BYTES(value)*(uint64_t)1024)
#define MEGABYTES(value) (KILOBYTES(value)*(uint64_t)1024)
#define GIGABYTES(value) (MEGABYTES(value)*(uint64_t)1024)

//constexpr int32_t ALIGNMENT = 64; 
  
using furious_alloc_t = void* (*) (void*,     // ptr to state
                                   uint32_t,   // alignment 
                                   uint32_t,   // size in bytes
                                   uint32_t);  // hint with the block id of this allocation. -1 indicates no hint

using furious_free_t = void (*) (void*,       // ptr to state
                                 void*);      // ptr to free



struct mem_allocator_t
{
  void*           p_mem_state;
  furious_alloc_t p_mem_alloc;
  furious_free_t  p_mem_free;
};

extern mem_allocator_t  global_mem_allocator;
extern mem_allocator_t  frame_mem_allocator; 

void* 
mem_alloc(mem_allocator_t* mem_allocator, 
          uint32_t alignment, 
          uint32_t size, 
          uint32_t hint);

void 
mem_free(mem_allocator_t* mem_allocator, 
         void* ptr);

/**
 * \brief Sets the memory allocator for furious
 *
 * \param allocator The memory allocator to set
 */
void
furious_set_mem_alloc(mem_allocator_t* allocator);

/**
 * \brief Sets the memory allocator used during frames 
 *
 * \param allocator The memory allocator to set
 */
void
furious_set_frame_mem_alloc(mem_allocator_t* allocator);

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


} /* furious */ 

#endif
