


#ifndef _FDB_TX_HEAP_ALLOCATOR_H_
#define _FDB_TX_HEAP_ALLOCATOR_H_

#include "tx.h"
#include "txpool_allocator.h"
#include "../../../common/types.h"
#include "../../../common/mutex.h"
#include "../../../common/btree.h"

#ifdef __cplusplus
extern "C" {
#endif


  struct fdb_txheap_alloc_ref_t
  {
    struct fdb_txpool_alloc_t*    p_alloc;  //< Pointer to the pool the reference belongs to
    struct fdb_txpool_alloc_ref_t m_ref;    //< The actual reference to the pool
  };

  struct fdb_txheap_alloc_t
  {
    struct fdb_btree_factory_t m_btree_factory;   //< Factory for the pools btree
    struct fdb_btree_t         m_pools;           //< The the btree with the heap pools
    uint32_t                   m_page_size;       //< The page size 
    uint32_t                   m_alignment;
    struct fdb_mem_allocator_t* p_allocator; 
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
  fdb_txheap_alloc_init(struct fdb_txheap_alloc_t* palloc, 
                        uint32_t alignment, 
                        uint32_t page_size, 
                        struct fdb_mem_allocator_t* allocator);


  /**
   * \brief releases a pool allocator
   *
   * \param fdb_txheap_alloc The pool allocator to release
   */
  void
  fdb_txheap_alloc_release(struct fdb_txheap_alloc_t* allocator, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx);


  /**
   * \brief  Allocates a block from the txheap.  
   *
   * \param palloc
   * \param tx
   * \param alignment
   * \param size
   * \param hint
   *
   * \return 
   */
  struct fdb_txheap_alloc_ref_t
  fdb_txheap_alloc_alloc(struct fdb_txheap_alloc_t* palloc, 
                         struct fdb_tx_t* tx,
                         struct fdb_txthread_ctx_t* txtctx,
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint);
  /**
   * \brief Nullifies a reference and frees the old blocks 
   *
   * \param palloc The txheap allocator the reference belongs to
   * \param tx The transaction that frees and nullifies the reference
   * \param txtctx The thread context
   * \param ref The reference to nullify
   */
  void
  fdb_txheap_alloc_free(struct fdb_txheap_alloc_t* palloc, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx,
                        struct fdb_txheap_alloc_ref_t ref);

  /**
   * \brief Gets the pointer of a txheap reference
   *
   * \param palloc The txheap the reference belongs to
   * \param tx The transaction
   * \param ref The reference to get the pointer from
   *
   * \return Returns the pointer of the reference
   */
  void*
  fdb_txheap_alloc_ptr(struct fdb_txheap_alloc_t* palloc, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       struct fdb_txheap_alloc_ref_t ref, 
                       bool write);

  /**
   * \brief Flushes the txheap allocator
   *
   * \param palloc The txheap allocator to flush
   * \param tx The transacion object
   * \param txtctx The thread context
   */
  void
  fdb_txheap_alloc_flush(struct fdb_txheap_alloc_t* palloc);

#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_txheap_ALLOCATOR_H_ */
