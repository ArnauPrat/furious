


#ifndef _FURIOUS_POOL_ALLOCATOR_H_
#define _FURIOUS_POOL_ALLOCATOR_H_ value

#include "memory.h"
#include "../types.h"
#include "../mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fdb_pool_alloc_header_t
{
  void*   p_next;
};

struct fdb_pool_alloc_t
{
  void*                   p_first_chunk; //< Pointer to the first memory chunk/page
  void*                   p_last_chunk;  //< Pointer to the last memory chunk/page
  void*                   p_first_free;  //< Pointer to the first block freed
  uint64_t                m_alignment;   //< The alignment of the blocks
  uint64_t                m_block_size;  //< The size of the allocated blocks
  uint64_t                m_page_size;   //< The size of the preallocation chunks
  uint64_t                m_next_free;   //< The next free offset of the last chunk
  uint64_t                m_grow_offset; //< The amount of offset to grow next_free after each allocation
  uint64_t                m_lost_bytes;  //< The amount of lost bytes per page due to growoffset unalignment plus header
  struct fdb_mem_allocator_t*    p_allocator;   //< The parent memory allocator
  struct fdb_mem_allocator_t     m_super;
  struct fdb_mem_stats_t         m_stats;
  struct fdb_mutex_t             m_mutex;
};

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
fdb_pool_alloc_init(struct fdb_pool_alloc_t* palloc, 
                    uint32_t alignment, 
                    uint32_t block_size, 
                    uint32_t page_size, 
                    struct fdb_mem_allocator_t* allocator);


/**
 * \brief releases a pool allocator
 *
 * \param fdb_pool_alloc The pool allocator to release
 */
void
fdb_pool_alloc_release(struct fdb_pool_alloc_t* allocator);

void* 
fdb_pool_alloc_alloc(struct fdb_pool_alloc_t* palloc, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint);
void
fdb_pool_alloc_free(struct fdb_pool_alloc_t* palloc, 
                    void* ptr);

void
fdb_pool_alloc_flush(struct fdb_pool_alloc_t* palloc);

struct fdb_mem_stats_t
fdb_pool_alloc_stats(struct fdb_pool_alloc_t* palloc);

#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_pool_ALLOCATOR_H_ */
