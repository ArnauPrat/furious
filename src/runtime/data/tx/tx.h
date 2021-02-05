
#ifndef _FDB_TX_H_
#define _FDB_TX_H_

#include "../../../common/platform.h"
#include "../../../common/atomic_counter.h"
#include "../../../common/memory/pool_allocator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct fdb_txpool_alloc_t;
  struct fdb_txpool_alloc_block_t;
  struct fdb_txthread_ctx_t;

#define FDB_TX_INVALID_ID (uint64_t)0LL

  enum fdb_txtype_t
  {
    E_READ_ONLY,      ///< A read-only transaction
    E_READ_WRITE      ///< A read-write transaction
  };

  struct fdb_tx_t
  {
    uint64_t                    m_id;           //< The id of the transaction 
    uint64_t                    m_txversion;    //< The maximum version timestamp
    uint64_t                    m_ortxversion;  //< The ts of the oldest running ts. Used for GC during executiong of READ_WRITE tx's. 
    enum fdb_txtype_t           m_tx_type;      //< The transaction type
    struct fdb_tx_t*            p_next;         //< Next transaction in the transaction list
    struct fdb_tx_t*            p_prev;         //< Previous transaction in the transaction list
    struct fdb_txthread_ctx_t*  p_first_ctx;     //< Pointer to the first context in the thread context list
    struct fdb_txthread_ctx_t*  p_last_ctx;     //< Pointer to the first context in the thread context list
  };

  /**
   * \brief Initializes the transactional subsytem
   *
   * \param The memory allocator used to allocate the pools.
   */
  void
  fdb_tx_init(struct fdb_mem_allocator_t* pool_allocator);

  /**
   * \brief Releases the transactional subsystem
   */
  void
  fdb_tx_release();

  /**
   * \brief Initializes a transaction. From this point, the transaction has
   * started. This operation blocks the execution if a read-write transaction is
   * already running.
   *
   * \param tx The transaction object to initialize
   * \param fdb_txtype_t The type of transaction
   */
  void
  fdb_tx_begin(struct fdb_tx_t* tx, 
               enum fdb_txtype_t txtype);

  /**
   * \brief Commits a transaction
   *
   * \param tx The transaction to commit
   */
  void 
  fdb_tx_commit(struct fdb_tx_t* tx); 

  /**
   * \brief Locks the tx manager so no transactions can beging
   */
  void
  fdb_tx_lock();

  /**
   * \brief UnLocks the tx manager 
   */
  void
  fdb_tx_unlock();


  /**
   * \brief Initializes the txthread context
   *
   * \param txtctx The txthread context to initialize
   * \param allocator Pointer to the memory allocator to use within the context
   */
  struct fdb_txthread_ctx_t*  
  fdb_tx_txthread_ctx_get(struct fdb_tx_t* tx, 
                          struct fdb_mem_allocator_t* allocator);

  /**
   * \brief Adds a txpool reference to the given txthread context
   *
   * \param txctx The txthread context to add the reference to
   * \param palloc The txpool alloc the reference belongs to
   * \param ref The reference to add
   * \param ts The timestamp of the transaction that marked this block as
   * deleted
   * \param release True if we want to fully release the reference.
   */
  void
  fdb_txthread_ctx_gc_block(struct fdb_txthread_ctx_t* txtctx, 
                            struct fdb_txpool_alloc_t* palloc, 
                            struct fdb_txpool_alloc_block_t* ref);

  /**
   * \brief Garbage collectes an object. Used to gc objects whose destruction
   * needs to happen after a block has been gced.
   *
   * \param txtctx The thread context
   * \param obj The obj to gc
   * \param destr The destructor to gc the object
   * \param ts The timestamp of the transaction that marked this object to be
   * gced
   */
  void
  fdb_txthread_ctx_gc_obj(struct fdb_txthread_ctx_t* txtctx, 
                          void* obj,
                          void (*destr)(void*),
                          uint64_t ts);




#ifdef __cplusplus
}
#endif

#endif
