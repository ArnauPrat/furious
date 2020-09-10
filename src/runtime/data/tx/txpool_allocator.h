


#ifndef _FDB_TX_POOL_ALLOCATOR_H_
#define _FDB_TX_POOL_ALLOCATOR_H_

#include "tx.h"
#include "../../../common/memory/pool_allocator.h"
#include "../../../common/memory/memory.h"
#include "../../../common/types.h"
#include "../../../common/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct fdb_txpool_alloc_ref_t
  {
    uint64_t                        m_ts;
    void*                           p_data;
    struct fdb_txpool_alloc_ref_t*  p_zombie; 
    uint64_t                        m_zts;
    struct fdb_txpool_alloc_ref_t*  p_next_ref;
    bool                            m_freed;
  } fdb_txpool_alloc_ref_t;


  typedef struct fdb_txpool_alloc_t
  {
    fdb_pool_alloc_t        m_palloc;
    uint64_t                m_data_offset; //< The offset from the start of each block where data starts 
    uint32_t                m_alignment;
    uint32_t                m_payload_size;
    uint32_t                m_block_size;  
    fdb_mutex_t             m_mutex;
  } fdb_txpool_alloc_t;

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
  fdb_txpool_alloc_init(fdb_txpool_alloc_t* palloc, 
                        uint32_t alignment, 
                        uint32_t block_size, 
                        uint32_t page_size, 
                        fdb_mem_allocator_t* allocator);


  /**
   * \brief releases a pool allocator
   *
   * \param fdb_txpool_alloc The pool allocator to release
   */
  void
  fdb_txpool_alloc_release(fdb_txpool_alloc_t* allocator);

  /**
   * \brief  Allocates a block from the txpool.  
   *
   * \param palloc
   * \param tx
   * \param alignment
   * \param size
   * \param hint
   *
   * \return 
   */
  fdb_txpool_alloc_ref_t* 
  fdb_txpool_alloc_alloc(fdb_txpool_alloc_t* palloc, 
                         fdb_tx_t* tx,
                         fdb_txthread_ctx_t* txtctx,
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint);
  /**
   * \brief Releases a block from the tx pool
   *
   * \param palloc
   * \param tx
   * \param ptr
   */
  void
  fdb_txpool_alloc_free(fdb_txpool_alloc_t* palloc, 
                        fdb_tx_t* tx, 
                        fdb_txthread_ctx_t* txtctx,
                        fdb_txpool_alloc_ref_t* ref);

  /**
   * \brief Gets the pointer of a txpool reference
   *
   * \param palloc The txpool the reference belongs to
   * \param tx The transaction
   * \param ref The reference to get the pointer from
   *
   * \return Returns the pointer of the reference
   */
  void*
  fdb_txpool_alloc_ptr(fdb_txpool_alloc_t* palloc, 
                 fdb_tx_t* tx, 
                 fdb_txthread_ctx_t* txtctx,
                 fdb_txpool_alloc_ref_t* ref, 
                 bool write);

  /**
   * \brief Flushes the txpool allocator
   *
   * \param palloc The txpool allocator to flush
   * \param tx The transacion object
   * \param txtctx The thread context
   */
  void
  fdb_txpool_alloc_flush(fdb_txpool_alloc_t* palloc);


  /**
   * \brief Garbage collects a reference previously freed
   *
   * \param palloc The txpool the reference belongs to
   * \param ref The reference to garbage collect
   * \param tv The transaction version that freed this reference
   * \param orv The version of the oldest running transaction
   *
   * \return True if the reference was effectively freed
   */
  bool
  fdb_txpool_alloc_gc_free(fdb_txpool_alloc_t* palloc, 
                           fdb_txpool_alloc_ref_t* ref, 
                           uint64_t tv, 
                           uint64_t orv);



#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_txpool_ALLOCATOR_H_ */
