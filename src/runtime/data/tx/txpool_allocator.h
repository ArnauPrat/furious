


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

  struct fdb_txpool_alloc_block_t;

  typedef struct fdb_txpool_alloc_ref_t
  {
    struct fdb_txpool_alloc_block_t* volatile p_main;
  } fdb_txpool_alloc_ref_t;


  typedef struct fdb_txpool_alloc_t
  {
    fdb_pool_alloc_t        m_data_palloc;      //< The normal pool allocator used to allocate blocks
    fdb_pool_alloc_t        m_block_palloc;      //< The normal pool allocator used to allocate blocks
    uint32_t                m_data_size;//< The payload size. This is the size of the data requested by the user (plus alignment overhead)
    uint32_t                m_data_alignment;
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
   * \brief Initializes a txpool alloc reference as a null reference 
   *
   * \param palloc The txpool alloc the reference belongs to
   * \param ref The reference to initialize
   */
  void
  fdb_txpool_alloc_ref_nullify(fdb_txpool_alloc_t* palloc, 
                               fdb_txpool_alloc_ref_t* ref);


  /**
   * \brief Checks if a reference is null 
   *
   * \param palloc The txpool alloc the reference belongs to
   * \param ref The reference to initialize
   */
  bool
  fdb_txpool_alloc_ref_isnull(fdb_txpool_alloc_t* palloc, 
                             fdb_txpool_alloc_ref_t* ref);


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
  void 
  fdb_txpool_alloc_alloc(fdb_txpool_alloc_t* palloc, 
                         fdb_tx_t* tx,
                         fdb_txthread_ctx_t* txtctx,
                         uint32_t alignment, 
                         uint32_t size,
                         uint32_t hint, 
                         fdb_txpool_alloc_ref_t* ref);
  /**
   * \brief Nullifies a reference and frees the old blocks 
   *
   * \param palloc The txpool allocator the reference belongs to
   * \param tx The transaction that frees and nullifies the reference
   * \param txtctx The thread context
   * \param ref The reference to nullify
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
  bool fdb_txpool_alloc_gc(fdb_txpool_alloc_t* palloc, 
                            fdb_txthread_ctx_t* txtctx,
                            uint64_t orv,
                            struct fdb_txpool_alloc_block_t* root, 
                            bool force);



#ifdef __cplusplus
}
#endif


#endif /* ifndef _FURIOUS_fdb_txpool_ALLOCATOR_H_ */
