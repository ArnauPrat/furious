


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

  struct fdb_txpool_alloc_t;
  struct fdb_txpool_alloc_block_t;

  struct fdb_txpool_alloc_ref_t
  {
    struct fdb_txpool_alloc_block_t* volatile p_main;
  };

  /**
   * \brief Creates and initializes a txpool allocator
   *
   * \param alignment The alignment of the allocations
   * \param block_size The size of the allocations
   * \param page_size The size of the batches to preallocate
   * \param allocator The parent allocator to use by this allocator
   *
   *
   * \return The created and initialized txpool allocator
   */
  struct fdb_txpool_alloc_t* 
  fdb_txpool_alloc_create(uint32_t alignment, 
                          uint32_t block_size, 
                          uint32_t page_size, 
                          struct fdb_mem_allocator_t* allocator);

  /**
   * \brief Deinitializes and destroys a txpool allocator
   *
   * \param fdb_txpool_alloc_t The txpool allocator to deinitialize and destroy
   */
  void
fdb_txpool_alloc_destroy(struct fdb_txpool_alloc_t* allocator, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx);


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
  struct fdb_txpool_alloc_ref_t
  fdb_txpool_alloc_alloc(struct fdb_txpool_alloc_t* palloc, 
                         struct fdb_tx_t* tx,
                         struct fdb_txthread_ctx_t* txtctx,
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint);
  /**
   * \brief Nullifies a reference and frees the old blocks 
   *
   * \param palloc The txpool allocator the reference belongs to
   * \param tx The transaction that frees and nullifies the reference
   * \param txtctx The thread context
   * \param ref The reference to nullify
   */
  void
  fdb_txpool_alloc_free(struct fdb_txpool_alloc_t* palloc, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx,
                        struct fdb_txpool_alloc_ref_t ref);

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
  fdb_txpool_alloc_ptr(struct fdb_txpool_alloc_t* palloc, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       struct fdb_txpool_alloc_ref_t ref, 
                       bool write);

  /**
   * \brief Flushes the txpool allocator
   *
   * \param palloc The txpool allocator to flush
   * \param tx The transacion object
   * \param txtctx The thread context
   */
  void
  fdb_txpool_alloc_flush(struct fdb_txpool_alloc_t* palloc);

  /**
   * \brief Garbage collects the stale blocks of a given reference of a tx pool
   *
   * \param palloc The txpool of the reference to garbage collect
   * \param txtctx The thread context executing the transaction  
   * \param orv The oldest running transaction version
   * \param root The root block to garbage collect
   * \param force Forces the GC process assuming there are no running txs 
   *
   * \return True if this reference is completely GCed
   */
  bool fdb_txpool_alloc_gc(struct fdb_txpool_alloc_t* palloc, 
                           struct fdb_txthread_ctx_t* txtctx,
                           uint64_t orv,
                            struct fdb_txpool_alloc_block_t* root, 
                            bool force);


  /**
   * \brief gets the timestamp of a block
   *
   * \param block The block to get the timestamp from
   *
   * \return The timetamp of the block
   */
  uint64_t
  fdb_txpool_alloc_block_ts(struct fdb_txpool_alloc_block_t* block);


#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_txpool_ALLOCATOR_H_ */
