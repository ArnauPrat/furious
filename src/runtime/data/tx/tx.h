
#ifndef _FDB_TX_H_
#define _FDB_TX_H_

#include "../../../common/platform.h"
#include "../../../common/memory/pool_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct fdb_txpool_alloc_t;
  struct fdb_txpool_alloc_ref_t;

  typedef enum fdb_txtype_t
  {
    E_READ_ONLY,      ///< A read-only transaction
    E_READ_WRITE      ///< A read-write transaction
  } fdb_txtype_t;

  typedef struct fdb_tx_t
  {
    uint64_t      m_id;         ///< The id of the transaction 
    fdb_txtype_t  m_tx_type;    ///< The transaction type
  } fdb_tx_t;

  typedef struct fdb_tx_garbage_t
  {
    struct fdb_txpool_alloc_t* p_palloc;  ///< The txpool where this garbage item belongs to
    struct fdb_txpool_ref_t*   p_ref;     ///< The reference to the garbage item
    struct fdb_txgarbage_t*   p_next;     ///< The next garbage entry in the linkect list
    struct fdb_txgarbage_t*   p_prev;     ///< The previous garbage entry in th elinked list
  } fdb_tx_garbage_t;


  typedef struct fdb_txthread_ctx_t
  {
    uint32_t            m_tid;          ///< the thread id
    uint32_t            m_nthreads;     ///< the number of threads (siblings) that will be used to execute transactions
    fdb_pool_alloc_t    m_palloc;       ///< the pool allocator used to allocate garbage objects
    fdb_tx_garbage_t*   p_first;        ///< the first garbage object in the linked list 
  } fdb_txthread_ctx_t;


  /**
   * \brief Initializes a transaction. From this point, the transaction has
   * started. This operation blocks the execution if a read-write transaction is
   * already running.
   *
   * \param tx The transaction object to initialize
   * \param fdb_txtype_t The type of transaction
   */
  void
  fdb_tx_begin(fdb_tx_t* tx, fdb_txtype_t txtype);

  /**
   * \brief Commits a transaction
   *
   * \param tx The transaction to commit
   */
  void 
  fdb_tx_commit(fdb_tx_t* tx); 


  /**
   * \brief Initializes the txthread context
   *
   * \param txtctx The txthread context to initialize
   * \param tid The thread id (0 to nthreads-1)
   * \param nthreads The total number of threads
   */
  void
  fdb_txthread_ctx_init(fdb_txthread_ctx_t* txtctx, 
                        uint32_t tid, 
                        uint32_t nthreads, 
                        fdb_mem_allocator_t* allocator);

  /**
   * \brief Releases the txthread context
   *
   * \param txctx The txthread context to release
   */
  void
  fdb_txthread_ctx_release(fdb_txthread_ctx_t* txctx);

  /**
   * \brief Adds a txpool reference to the given txthread context
   *
   * \param txctx The txthread context to add the reference to
   * \param palloc The txpool alloc the reference belongs to
   * \param ref The reference to add
   */
  void
  fdb_txthread_ctx_add_entry(fdb_txthread_ctx_t* txtctx, 
                             struct fdb_txpool_alloc_t* palloc, 
                             struct fdb_txpool_alloc_ref_t* ref);

  /**
   * \brief Executes the garbage collection process on this txthread ctx.
   *
   * \param txtctx The txthread context to run the garbage collection process
   * for
   */
  void
  fdb_txthread_ctx_gc(fdb_txthread_ctx_t* txtctx);

#ifdef __cplusplus
}
#endif

#endif
