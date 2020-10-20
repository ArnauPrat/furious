

#include "tx.h"
#include "../../../common/mutex.h"
#include "txpool_allocator.h"
#include <stdbool.h>

static fdb_tx_t*         p_first = NULL;
static fdb_tx_t*         p_last = NULL;
static fdb_mutex_t       m_mutex;
static uint64_t          m_next_id = 1;
static uint64_t          m_last_write_committed_id = 0;
static fdb_mutex_t       m_write_running;

void
fdb_tx_init()
{
  fdb_mutex_init(&m_mutex);
  fdb_mutex_init(&m_write_running);
  m_next_id = 1;
  m_last_write_committed_id = 0;
  FDB_ASSERT(!p_first)
  FDB_ASSERT(!p_last)
  p_first = NULL;
  p_last = NULL;
}

void
fdb_tx_release()
{
  FDB_ASSERT(!p_first)
  FDB_ASSERT(!p_last)
  m_next_id = 1;
  m_last_write_committed_id = 0;
  p_first = NULL;
  p_last = NULL;
  fdb_mutex_release(&m_write_running);
  fdb_mutex_release(&m_mutex);
}

void
fdb_tx_begin(fdb_tx_t* tx, fdb_txtype_t txtype)
{
  if(txtype == E_READ_WRITE)
  {
    fdb_mutex_lock(&m_write_running);
  }

  fdb_mutex_lock(&m_mutex);
  *tx = (fdb_tx_t){.m_id = m_next_id++, 
                   .m_txversion = m_last_write_committed_id,
                   .m_tx_type = txtype, 
                   .p_next = NULL, 
                   .p_prev = NULL};

  if(tx->m_tx_type == E_READ_WRITE)
  {
    tx->m_txversion = tx->m_id;
  }

  if(p_first == NULL)
  {
    tx->m_ortxversion = m_last_write_committed_id;
    FDB_ASSERT(p_last == NULL);
    p_first = tx;
    p_last = tx;
  }
  else
  {
    tx->m_ortxversion = p_first->m_txversion;
    p_last->p_next = tx;
    tx->p_prev = p_last;
    p_last = tx;
  }
  fdb_mutex_unlock(&m_mutex);
}

void
fdb_tx_commit(fdb_tx_t* tx)
{
  fdb_mutex_lock(&m_mutex);
  if(tx->p_prev != NULL)
  {
    tx->p_prev->p_next = tx->p_next;
  }
  if(tx->p_next != NULL)
  {
    tx->p_next->p_prev = tx->p_prev;
  }

  if(tx->m_tx_type == E_READ_WRITE &&
     tx->m_id > m_last_write_committed_id)
  {
    m_last_write_committed_id = tx->m_id;
  }

  if(p_first == tx)
  {
    p_first = tx->p_next;
  }

  if(p_last == tx)
  {
    p_last = tx->p_prev;
  }

  tx->p_next = NULL;
  tx->p_prev = NULL;

  fdb_mutex_unlock(&m_mutex);

  if(tx->m_tx_type == E_READ_WRITE)
  {
    fdb_mutex_unlock(&m_write_running);
  }
}


void
fdb_txthread_ctx_init(fdb_txthread_ctx_t* txtctx, 
                      fdb_mem_allocator_t* allocator)
{
  *txtctx = (fdb_txthread_ctx_t){.p_first = NULL};
  fdb_pool_alloc_init(&txtctx->m_palloc, 
                      FDB_MIN_ALIGNMENT, 
                      sizeof(fdb_txgarbage_t), 
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
fdb_txthread_ctx_add_entry(fdb_txthread_ctx_t* txtctx, 
                           struct fdb_txpool_alloc_t* palloc, 
                           struct fdb_txpool_alloc_ref_t* ref)
{

  fdb_txgarbage_t* entry  = (fdb_txgarbage_t*)fdb_pool_alloc_alloc(&txtctx->m_palloc,
                                                                   FDB_MIN_ALIGNMENT, 
                                                                   sizeof(fdb_txgarbage_t), 
                                                                   FDB_NO_HINT);
  *entry = (fdb_txgarbage_t){0};
  if(txtctx->p_first != NULL)
  {
    txtctx->p_first->p_prev = entry;
    entry->p_next = txtctx->p_first;
  }
  txtctx->p_first = entry;
  entry->p_palloc = palloc;
  entry->p_ref = ref;
}

bool
fdb_txthread_ctx_gc(fdb_txthread_ctx_t* txtctx, bool force)
{
  bool all_freed = true;
  fdb_mutex_lock(&m_mutex);
  uint64_t oldest_running_version = m_last_write_committed_id;
  if(p_first != NULL)
  {
    oldest_running_version = p_first->m_txversion;
  }
  fdb_mutex_unlock(&m_mutex);

  if(force)
  {
    fdb_mutex_lock(&m_mutex);
  }
  fdb_txgarbage_t* next_garbage = txtctx->p_first;
  while(next_garbage != NULL)
  {
    if(fdb_txpool_alloc_gc(next_garbage->p_palloc, 
                           txtctx,
                           oldest_running_version, 
                           next_garbage->p_ref, 
                           force))
    {
      if(next_garbage == txtctx->p_first)
      {
        txtctx->p_first = next_garbage->p_next;
      }

      if(next_garbage->p_prev)
      {
        next_garbage->p_prev->p_next = next_garbage->p_next;
      }

      if(next_garbage->p_next)
      {
        next_garbage->p_next->p_prev = next_garbage->p_prev;
      }

      fdb_txgarbage_t* gb = next_garbage;
      next_garbage = next_garbage->p_next;
      fdb_pool_alloc_free(&txtctx->m_palloc, gb);
    }
    else
    {
      all_freed = false;
      next_garbage = next_garbage->p_next;
    }
  }

  if(force)
  {
    fdb_mutex_unlock(&m_mutex);
  }

  return all_freed;
}

void
fdb_tx_lock()
{
  fdb_mutex_lock(&m_mutex);
}

void
fdb_tx_unlock()
{
  fdb_mutex_unlock(&m_mutex);
}
