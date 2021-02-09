

#include "tx.h"
#include "../../../common/mutex.h"
#include "../../../common/memory/pool_allocator.h"
#include "txpool_allocator.h"
#include <stdbool.h>

static struct fdb_tx_t*               p_first = NULL;
static struct fdb_tx_t*               p_last = NULL;
static struct fdb_mutex_t             m_mutex;
static uint64_t                       m_next_id = 1;
static uint64_t                       m_last_write_committed_id = 0;
static struct fdb_mutex_t             m_write_running;
static struct fdb_pool_alloc_t        m_txtctx_alloc; 
static struct fdb_txthread_ctx_t*      p_first_ctx;
static struct fdb_txthread_ctx_t*      p_last_ctx;

enum fdb_txgarbage_type_t
{
  E_BLOCK, 
  E_OBJ,
};

struct fdb_txgarbage_t
{
  enum fdb_txgarbage_type_t              m_type;
  union {
    struct {
      struct fdb_txpool_alloc_t*        p_palloc;   ///< The txpool where this garbage item belongs to
      struct fdb_txpool_alloc_block_t*  p_block;    ///< The block to the garbage
    } m_block;
    struct {
      void*                             p_obj;   ///< The txpool where this garbage item belongs to.
      void                              (*p_destructor)(void*);
      uint64_t                          m_ts;       ///< TS of the transaction that added this allocator 
    } m_obj;
  };
  struct fdb_txgarbage_t*           p_next;     ///< The next garbage entry in the linkect list
  struct fdb_txgarbage_t*           p_prev;     ///< The previous garbage entry in th elinked list
};

struct fdb_txthread_ctx_t
{
  struct fdb_tx_t*           p_tx;          //< The transaction this context belongs to
  struct fdb_pool_alloc_t    m_palloc;      //< the pool allocator used to allocate garbage objects
  struct fdb_txgarbage_t*    p_first;       //< the first garbage object in the linked list 
  struct fdb_txgarbage_t*    p_last;        //< the last garbage object in the linked list 
  struct fdb_txthread_ctx_t* p_next;        //< The next ctx in the list
  struct fdb_txthread_ctx_t* p_prev;        //< The previous ctx in the list
};


void fdb_txpool_alloc_init_subsystem(struct fdb_mem_allocator_t* allocator);
void fdb_txpool_alloc_release_subsystem();

/**
 * \brief Unlinks a transaction thread context
 *
 * \param txtctx The transaction thread context to unlink
 */
static void 
fdb_txthread_ctx_unlink(struct fdb_txthread_ctx_t* txtctx)
{
  if(txtctx->p_prev != NULL)
  {
    txtctx->p_prev->p_next = txtctx->p_next;
  }

  if(txtctx->p_next!= NULL)
  {
    txtctx->p_next->p_prev = txtctx->p_prev;
  }

  txtctx->p_next= NULL;
  txtctx->p_prev= NULL;
}

/**
 * \brief Release a transaction thread context
 *
 * \param txtctx The transaction thread context to release
 */
static void 
fdb_txthread_ctx_release(struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(txtctx->p_first == NULL && "Cannot release a non GCed transaction thead context")
  fdb_pool_alloc_release(&txtctx->m_palloc);
  fdb_pool_alloc_free(&m_txtctx_alloc, txtctx);
}

/**
 * \brief Executes the garbage collection process on this txthread ctx.
 *
 * \param txtctx The txthread context to run the garbage collection process
 * for
 * \param force Locks the tx manager and forces the garbage collection process.  
 *
 * \return True if all items were garbage collected. False otherwise
 */
static bool
fdb_txthread_ctx_gc(struct fdb_txthread_ctx_t* txtctx, bool force)
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
  struct fdb_txgarbage_t* next_garbage = txtctx->p_first;
  while(next_garbage != NULL)
  {
    bool collected = false;
    if(next_garbage->m_type == E_BLOCK)
    {
      collected = fdb_txpool_alloc_gc(next_garbage->m_block.p_palloc, 
                                      txtctx,
                                      oldest_running_version, 
                                      next_garbage->m_block.p_block, 
                                      force);
    }
    else
    {
      if(next_garbage->m_obj.m_ts < oldest_running_version || force)
      {
        next_garbage->m_obj.p_destructor(next_garbage->m_obj.p_obj);
        collected = true;
      }
    }

    if(collected)
    {
      if(next_garbage == txtctx->p_first)
      {
        txtctx->p_first = next_garbage->p_next;
      }

      if(next_garbage == txtctx->p_last)
      {
        txtctx->p_last = next_garbage->p_prev;
      }

      if(next_garbage->p_prev)
      {
        next_garbage->p_prev->p_next = next_garbage->p_next;
      }

      if(next_garbage->p_next)
      {
        next_garbage->p_next->p_prev = next_garbage->p_prev;
      }

      struct fdb_txgarbage_t* gb = next_garbage;
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

static void
fdb_tx_do_gc(bool flush)
{
    // GCing global gc list
    struct fdb_txthread_ctx_t* next = p_first_ctx;
    while(next != NULL)
    {
      struct fdb_txthread_ctx_t* aux = next->p_next;
      fdb_txthread_ctx_gc(next, flush);
      if(next->p_first == NULL)
      {
        if(next == p_first_ctx)
        {
          p_first_ctx = aux;
        }

        if(next == p_last_ctx)
        {
          p_last_ctx = next->p_prev;
        }

        fdb_txthread_ctx_unlink(next);
        fdb_txthread_ctx_release(next);
      }
      next = aux;
    }
}

void
fdb_tx_init(struct fdb_mem_allocator_t* allocator)
{
  fdb_mutex_init(&m_mutex);
  fdb_mutex_init(&m_write_running);
  m_next_id = 1;
  m_last_write_committed_id = 0;
  FDB_ASSERT(!p_first)
  FDB_ASSERT(!p_last)
  p_first = NULL;
  p_last = NULL;
  p_first_ctx = NULL;
  p_last_ctx = NULL;
  fdb_pool_alloc_init(&m_txtctx_alloc, 
                      FDB_TX_CTX_ALIGNMENT, 
                      sizeof(struct fdb_txthread_ctx_t), 
                      FDB_TX_CTX_PAGE_SIZE, 
                      allocator);
  fdb_txpool_alloc_init_subsystem(allocator);
}

void
fdb_tx_release()
{
  FDB_ASSERT(!p_first && "There are uncommitted transactions")
  FDB_ASSERT(!p_last)
  fdb_tx_do_gc(true);
  FDB_ASSERT(!p_first_ctx);
  FDB_ASSERT(!p_last_ctx);
  fdb_txpool_alloc_release_subsystem();
  fdb_pool_alloc_release(&m_txtctx_alloc);
  m_next_id = 1;
  m_last_write_committed_id = 0;
  p_first = NULL;
  p_last = NULL;
  p_first_ctx = NULL;
  p_last_ctx = NULL;
  fdb_mutex_release(&m_write_running);
  fdb_mutex_release(&m_mutex);
}

void
fdb_tx_begin(struct fdb_tx_t* tx, enum fdb_txtype_t txtype)
{
  if(txtype == E_READ_WRITE)
  {
    fdb_mutex_lock(&m_write_running);
  }

  fdb_mutex_lock(&m_mutex);
  *tx = (struct fdb_tx_t){.m_id = m_next_id++, 
                   .m_txversion = m_last_write_committed_id,
                   .m_tx_type = txtype, 
                   .p_next = NULL, 
                   .p_prev = NULL, 
                   .p_first_ctx = NULL, 
                   .p_last_ctx = NULL};

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
fdb_tx_commit(struct fdb_tx_t* tx)
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
    // Gcing global list
    fdb_tx_do_gc(false);

    // GCing local gc list
    struct fdb_txthread_ctx_t* next = tx->p_first_ctx;
    while(next != NULL)
    {
      fdb_txthread_ctx_gc(next, false);
      next = next->p_next;
    }

    next = tx->p_first_ctx;
    while(next != NULL)
    {
      struct fdb_txthread_ctx_t* aux = next->p_next;
      if(next->p_first == NULL)
      {
        // No need to update tx->p_first_ctx and tx->p_last_ctx since tx will be
        // removed
        fdb_txthread_ctx_unlink(next);
        fdb_txthread_ctx_release(next);
      }
      else
      {
        fdb_txthread_ctx_unlink(next);
        if(p_first_ctx == NULL)
        {
          FDB_ASSERT(p_last_ctx == NULL);
          p_first_ctx = next;
          p_last_ctx = next;
        }
        else
        {
          next->p_prev = p_last_ctx;
          p_last_ctx->p_next = next;
          p_last_ctx = next;
        }
      }
      next = aux;
    }

    fdb_mutex_unlock(&m_write_running);
  }
  else
  {
    struct fdb_txthread_ctx_t* next = tx->p_first_ctx;
    while(next != NULL)
    {
      FDB_ASSERT(next->p_first == NULL && "Context from a READ transaction should be null");
      fdb_pool_alloc_release(&next->m_palloc);
      struct fdb_txthread_ctx_t* aux = next->p_next;
      fdb_pool_alloc_free(&m_txtctx_alloc, next);
      next = aux;
    }
  }

}


struct fdb_txthread_ctx_t*
fdb_tx_txthread_ctx_get(struct fdb_tx_t* tx, 
                        struct fdb_mem_allocator_t* allocator)
{
  struct fdb_txthread_ctx_t* txtctx = fdb_pool_alloc_alloc(&m_txtctx_alloc, 
                                                           FDB_TX_CTX_ALIGNMENT, 
                                                           sizeof(struct fdb_txthread_ctx_t), 
                                                           FDB_NO_HINT);
  *txtctx = (struct fdb_txthread_ctx_t){};
  txtctx->p_tx = tx;
  fdb_pool_alloc_init(&txtctx->m_palloc, 
                      FDB_MIN_ALIGNMENT, 
                      sizeof(struct fdb_txgarbage_t), 
                      FDB_TX_GC_PAGE_SIZE,
                      allocator); 
  if(tx->p_first_ctx == NULL)
  {
    FDB_ASSERT(tx->p_last_ctx == NULL);
    tx->p_first_ctx = txtctx;
    tx->p_last_ctx = txtctx;
  }
  else
  {
    FDB_ASSERT(tx->p_last_ctx != NULL);
    txtctx->p_prev = tx->p_last_ctx;
    tx->p_last_ctx->p_next = txtctx;
    tx->p_last_ctx = txtctx;
  }
  return txtctx;
}

void
fdb_txthread_ctx_gc_block(struct fdb_txthread_ctx_t* txtctx, 
                          struct fdb_txpool_alloc_t* palloc, 
                          struct fdb_txpool_alloc_block_t* block)
{
  struct fdb_txgarbage_t* entry  = (struct fdb_txgarbage_t*)fdb_pool_alloc_alloc(&txtctx->m_palloc,
                                                                                 FDB_MIN_ALIGNMENT, 
                                                                                 sizeof(struct fdb_txgarbage_t), 
                                                                                 FDB_NO_HINT);
  *entry = (struct fdb_txgarbage_t){0};
  if(txtctx->p_first != NULL)
  {
    FDB_ASSERT(txtctx->p_last != NULL && "Pointer to last cannot be NULL");
    // Looking for place to insert the block based on its timestamp
    struct fdb_txgarbage_t* next = txtctx->p_first;
    while(fdb_txpool_alloc_block_ts(next->m_block.p_block) <= fdb_txpool_alloc_block_ts(block))
    {
      if(next->p_next == NULL)
        break;
      next = next->p_next;
    }
    if(next->p_next != NULL)
    {
      next->p_next->p_prev= entry;
      entry->p_next = next->p_next;
    }
    next->p_next = entry;
    entry->p_prev = next;
    if(txtctx->p_last == next)
    {
      txtctx->p_last = entry;
    }
  }
  else
  {
    txtctx->p_first = entry;
    txtctx->p_last = entry;
    entry->p_next = NULL;
    entry->p_prev = NULL;
  }

  entry->m_type = E_BLOCK;
  entry->m_block.p_palloc = palloc;
  entry->m_block.p_block = block;
}

void
fdb_txthread_ctx_gc_obj(struct fdb_txthread_ctx_t* txtctx, 
                        void* obj, 
                        void (*destr)(void*),
                        uint64_t ts)
{

  struct fdb_txgarbage_t* entry  = (struct fdb_txgarbage_t*)fdb_pool_alloc_alloc(&txtctx->m_palloc,
                                                                                 FDB_MIN_ALIGNMENT, 
                                                                                 sizeof(struct fdb_txgarbage_t), 
                                                                                 FDB_NO_HINT);
  *entry = (struct fdb_txgarbage_t){0};
  if(txtctx->p_first != NULL)
  {
    FDB_ASSERT(txtctx->p_last != NULL && "Pointer to last cannot be NULL");
    txtctx->p_last->p_next = entry;
    entry->p_prev = txtctx->p_last;
    entry->p_next = NULL;
    txtctx->p_last = entry;
  }
  else
  {
    txtctx->p_first = entry;
    txtctx->p_last = entry;
    entry->p_next = NULL;
    entry->p_prev = NULL;
  }

  entry->m_type = E_OBJ;
  entry->m_obj.p_obj = obj;
  entry->m_obj.p_destructor = destr;
  entry->m_obj.m_ts = ts;
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
