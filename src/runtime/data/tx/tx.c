


#include "tx.h"

void
fdb_txthread_ctx_init(fdb_txthread_ctx_t* txtctx, 
                      uint32_t tid, 
                      uint32_t nthreads, 
                      fdb_mem_allocator_t* allocator)
{
  *txtctx = (fdb_txthread_ctx_t){.m_tid = tid, 
                                 .m_nthreads = nthreads, 
                                 .p_first = NULL};
  fdb_pool_alloc_init(&txtctx->m_palloc, 
                      FDB_MIN_ALIGNMENT, 
                      sizeof(fdb_tx_garbage_t), 
                      FDB_TX_GC_PAGE_SIZE,
                      allocator);
}

void
fdb_txthread_ctx_release(fdb_txthread_ctx_t* txctx)
{
  FDB_ASSERT(txctx->p_first == NULL && "Cannot release txthread context with garbage pending to collect");
  fdb_pool_alloc_release(&txctx->m_palloc);
}

void
fdb_txthread_ctx_add_entry(fdb_txthread_ctx_t* txctx, 
                           struct fdb_txpool_alloc_t* palloc, 
                           struct fdb_txpool_alloc_ref_t* ref)
{
}
