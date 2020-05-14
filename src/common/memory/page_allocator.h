

#ifndef _FDB_PAGE_ALLOC_H_
#define _FDB_PAGE_ALLOC_H_

#include "memory.h"
#include "../mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_page_alloc_t
{
  void*               p_data;
  void*               p_stop;
  void*               p_ltop;
  void*               p_snextfree;
  void*               p_lnextfree;
  uint64_t            m_sregion;
  uint32_t            m_ssize;
  uint32_t            m_lsize;
  fdb_mem_allocator_t m_super;
  fdb_mem_stats_t     m_stats;
  fdb_mutex_t         m_mutex;
} fdb_page_alloc_t;

/**
 * \brief Initializes a page allocator
 *
 * \param palloc The page allocator to initialize
 * \param sregion The size of the memory region (in bytes) to allocate
 * \param ssmall The size of the small pages to allocate
 * \param slarge The size of the large pages to allocate
 */
void
fdb_page_alloc_init(fdb_page_alloc_t* palloc, 
                    uint64_t sregion,
                    uint32_t ssmall, 
                    uint32_t slarge);

/**
 * \brief Releases a page allocator
 *
 * \param palloc The page allocator to release
 */
void
fdb_page_alloc_release(fdb_page_alloc_t* palloc);

/**
 * \brief Allocates a block of memory using the page allocator. This block
 * of memory must be either of size ssmall or slarge. Otherwise the program will
 * abort
 *
 * \param palloc The page allocator
 * \param alignment The alignment of the allocation (ignored)
 * \param size The size of the allocation (either ssmall or slarge)
 * \param hint The hint passed to the allocator (ignored)
 */
void*
fdb_page_alloc_alloc(fdb_page_alloc_t* palloc, 
                     uint32_t alignment, 
                     uint32_t size,
                     uint32_t hint);

/**
 * \brief Frees a memory block from a page allocator
 *
 * \param palloc The page allocator to free the memory block from
 * \param ptr The pointer to the memory block to free
 */
void
fdb_page_alloc_free(fdb_page_alloc_t* palloc, 
                    void* ptr);

fdb_mem_stats_t
fdb_page_alloc_stats(fdb_page_alloc_t* palloc);

#ifdef __cplusplus
}
#endif

#endif
