


#ifndef _FURIOUS_POOL_ALLOCATOR_H_
#define _FURIOUS_POOL_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "../../common/memory/memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_pool_alloc_header_t
{
  void*   p_next;
} fdb_pool_alloc_header_t;

typedef struct fdb_pool_alloc_t
{
  void*                   p_first_chunk; //< Pointer to the first memory chunk
  void*                   p_last_chunk;  //< Pointer to the last memory chunk
  void*                   p_first_free;  //< Pointer to the first block freed
  uint64_t                m_alignment;   //< The alignment of the blocks
  uint64_t                m_block_size;  //< The size of the allocated blocks
  uint64_t                m_page_size;   //< The size of the preallocation chunks
  uint64_t                m_next_free;   //< The next free offset of the last chunk
  uint64_t                m_grow_offset; //< The amount of offset to grow next_free after each allocation
  fdb_mem_allocator_t*    p_allocator;   //< The parent memory allocator
  fdb_mem_allocator_t     m_super;
} fdb_pool_alloc_t;

/**
 * \brief inits a pool allocator
 *
 * \param alignment The alignment of the allocations
 * \param block_size The size of the allocations
 * \param page_size The size of the batches to preallocate
 * \param allocator The parent allocator to use by this allocator
 *
 * \return  Returns the memory allocator
 */
void
fdb_pool_alloc_init(fdb_pool_alloc_t* palloc, 
                    uint32_t alignment, 
                    uint32_t block_size, 
                    uint32_t page_size, 
                    fdb_mem_allocator_t* allocator);


/**
 * \brief releases a pool allocator
 *
 * \param fdb_pool_alloc The pool allocator to release
 */
void
fdb_pool_alloc_release(fdb_pool_alloc_t* allocator);

void* 
fdb_pool_alloc_alloc(fdb_pool_alloc_t* palloc, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint);
void
fdb_pool_alloc_free(fdb_pool_alloc_t* palloc, 
                    void* ptr);

void
fdb_pool_alloc_flush(fdb_pool_alloc_t* fdb_pool_alloc);

#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_pool_ALLOCATOR_H_ */
