

#include "txpool_allocator.h"
#include "../../../common/platform.h"
#include "string.h"

struct fdb_pool_alloc_t     m_tx_pool_allocator;

struct fdb_txpool_alloc_t
{
  struct fdb_pool_alloc_t        m_data_palloc;      //< The normal pool allocator used to allocate blocks
  struct fdb_pool_alloc_t        m_block_palloc;      //< The normal pool allocator used to allocate blocks
  uint32_t                m_data_size;//< The payload size. This is the size of the data requested by the user (plus alignment overhead)
  uint32_t                m_data_alignment;
};

typedef struct fdb_txpool_alloc_block_t
{
  uint64_t                                    m_ts;
  void*                                       p_data;
  struct fdb_txpool_alloc_block_t* volatile   p_next_version;
} fdb_txpool_alloc_block_t;


uint64_t
fdb_txpool_alloc_block_ts(struct fdb_txpool_alloc_block_t* block)
{
  return block->m_ts;
}

void
fdb_txpool_alloc_init(struct fdb_txpool_alloc_t* palloc, 
                      uint32_t alignment, 
                      uint32_t block_size, 
                      uint32_t page_size,
                      struct fdb_mem_allocator_t* allocator);
void
fdb_txpool_alloc_release(struct fdb_txpool_alloc_t* palloc);

void fdb_txpool_alloc_init_subsystem(struct fdb_mem_allocator_t* allocator)
{
  fdb_pool_alloc_init(&m_tx_pool_allocator, 
                      FDB_TXPOOL_POOL_ALIGNMENT, 
                      sizeof(struct fdb_txpool_alloc_t), 
                      FDB_TXPOOL_POOL_PAGE_SIZE,
                      allocator);
}



void fdb_txpool_alloc_release_subsystem()
{
  fdb_pool_alloc_release(&m_tx_pool_allocator);
}


struct fdb_txpool_alloc_t* 
fdb_txpool_alloc_create(uint32_t alignment, 
                        uint32_t block_size, 
                        uint32_t page_size, 
                        struct fdb_mem_allocator_t* allocator)
{
  struct fdb_txpool_alloc_t* alloc = fdb_pool_alloc_alloc(&m_tx_pool_allocator, 
                                                          FDB_TXPOOL_POOL_ALIGNMENT, 
                                                          sizeof(struct fdb_txpool_alloc_t), 
                                                          FDB_NO_HINT);
  fdb_txpool_alloc_init(alloc, 
                        alignment, 
                        block_size, 
                        page_size, 
                        allocator);
  return alloc;
}


static void fdb_txpool_alloc_destr(void* pool)
{
  fdb_txpool_alloc_release((struct fdb_txpool_alloc_t*)pool);
}

void
fdb_txpool_alloc_destroy(struct fdb_txpool_alloc_t* allocator, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx)
{

  fdb_txthread_ctx_gc_obj(txtctx, 
                          allocator, 
                          fdb_txpool_alloc_destr, 
                          tx->m_txversion);
}


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
fdb_txpool_alloc_init(struct fdb_txpool_alloc_t* palloc, 
                      uint32_t alignment, 
                      uint32_t block_size, 
                      uint32_t page_size,
                      struct fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  memset(palloc, 0, sizeof(struct fdb_txpool_alloc_t));
  palloc->m_data_size = block_size;
  palloc->m_data_alignment = alignment;

  fdb_pool_alloc_init(&palloc->m_data_palloc, 
                      alignment, 
                      palloc->m_data_size, 
                      page_size, 
                      allocator);

  fdb_pool_alloc_init(&palloc->m_block_palloc, 
                      FDB_MIN_ALIGNMENT, 
                      sizeof(fdb_txpool_alloc_block_t), 
                      page_size, 
                      allocator);
}

/**
 * \brief releases a pool allocator
 *
 * \param fdb_txpool_alloc The pool allocator to release
 */
void
fdb_txpool_alloc_release(struct fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_release(&palloc->m_data_palloc);
  fdb_pool_alloc_release(&palloc->m_block_palloc);
}

void
fdb_txpool_alloc_flush(struct fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_flush(&palloc->m_data_palloc);
  fdb_pool_alloc_flush(&palloc->m_block_palloc);
}


/**
 * @brief Allocates a memory block in a numa node
 *
 * @param ptr The state
 * @param alignment The alignment of the memory allocator 
 * @param size The size in bytes of the memory block to allocate
 * @param hint The hint to the allocator. It corresponds to id of the allocated
 * block
 *
 * @return 
 */
struct fdb_txpool_alloc_ref_t 
fdb_txpool_alloc_alloc(struct fdb_txpool_alloc_t* palloc, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       uint32_t alignment, 
                       uint32_t size,
                       uint32_t hint)
{
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "Read transactions cannot allocate from a transactional pool");
  FDB_ASSERT(size == palloc->m_data_size);
  FDB_ASSERT(alignment == palloc->m_data_alignment);

  struct fdb_txpool_alloc_ref_t ref  = {};
  ref.p_main = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                     FDB_MIN_ALIGNMENT, 
                                     sizeof(fdb_txpool_alloc_block_t), 
                                     FDB_NO_HINT);
  *ref.p_main = (fdb_txpool_alloc_block_t){};
  ref.p_main->p_data = fdb_pool_alloc_alloc(&palloc->m_data_palloc, 
                                             palloc->m_data_alignment, 
                                             palloc->m_data_size, 
                                             hint);
  ref.p_main->m_ts = tx->m_txversion;
  return ref;
}


/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void fdb_txpool_alloc_free(struct fdb_txpool_alloc_t* palloc, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           struct fdb_txpool_alloc_ref_t ref)
{

  FDB_ASSERT(ref.p_main != NULL && "Cannot free a nullified reference");
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "A read-only tx cannot free a block");

  fdb_txpool_alloc_gc(palloc, 
                      txtctx,
                      tx->m_ortxversion,
                      ref.p_main, 
                      false);

  fdb_txpool_alloc_block_t* candidate_version = ref.p_main->p_next_version;
  /*while(candidate_version != NULL && 
    candidate_version->m_ts > tx->m_txversion)
    {
    candidate_version = candidate_version->p_next_version;
    }*/

  if(candidate_version == NULL)
  {
    candidate_version = ref.p_main;
  }

  FDB_ASSERT((candidate_version->m_ts <= tx->m_txversion && candidate_version->p_data != NULL) && "Detected double free");


  if(candidate_version->m_ts <= tx->m_ortxversion)
  {
    fdb_txpool_alloc_block_t* new_block = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                                               FDB_MIN_ALIGNMENT, 
                                                               sizeof(fdb_txpool_alloc_block_t), 
                                                               FDB_NO_HINT);
    *new_block = (fdb_txpool_alloc_block_t){};
    if(candidate_version != ref.p_main)
    {
      new_block->p_next_version = candidate_version->p_next_version;
    }
    else
    {
      new_block->p_next_version=NULL;
    }

    new_block->m_ts = tx->m_txversion;
    fdb_mem_barrier();
    ref.p_main->p_next_version = new_block; 
  }
  else
  {
    FDB_ASSERT(candidate_version->m_ts == tx->m_txversion);
    fdb_pool_alloc_free(&palloc->m_data_palloc, 
                        candidate_version->p_data);
    candidate_version->p_data = NULL;
  }

  fdb_txthread_ctx_gc_block(txtctx, 
                            palloc, 
                            ref.p_main);
}


void*
fdb_txpool_alloc_ptr(struct fdb_txpool_alloc_t* palloc, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx,
                     struct fdb_txpool_alloc_ref_t ref, 
                     bool write)
{

  FDB_ASSERT(ref.p_main != NULL && "Cannot dereference a nullified reference");

  if(tx->m_tx_type == E_READ_WRITE)
  {
    // Garbage collect stale blocks
    fdb_txpool_alloc_gc(palloc, 
                        txtctx,
                        tx->m_ortxversion,
                        ref.p_main, 
                        false);
  }


  void* ret = NULL;
  fdb_txpool_alloc_block_t* candidate_version = ref.p_main->p_next_version;
  while(candidate_version != NULL && 
        candidate_version->m_ts > tx->m_txversion)
  {

    FDB_ASSERT(candidate_version->p_next_version == NULL || 
               (candidate_version->m_ts > candidate_version->p_next_version->m_ts && "Invariant violation of decreasing order of ts in chain" ));
    candidate_version = candidate_version->p_next_version;
  }

  if(candidate_version == NULL)
  {
    candidate_version = ref.p_main;
  }

  if(candidate_version->m_ts > tx->m_txversion ||
     candidate_version->p_data == NULL) // This tx cannot still see this verstion 
  {
    FDB_ASSERT(false && "This transaction should not dereference this reference");
  }

  switch(tx->m_tx_type)
  {
    case E_READ_ONLY:
      {
        ret = candidate_version->p_data;
      }
      break;
    case E_READ_WRITE:
      {
        if(write && candidate_version->m_ts < tx->m_txversion)
        {
          FDB_ASSERT((candidate_version == ref.p_main || candidate_version == ref.p_main->p_next_version) && "Invariant violation. Write transactions should always get most recent version");

          fdb_txpool_alloc_block_t* new_block = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                                                     FDB_MIN_ALIGNMENT, 
                                                                     sizeof(fdb_txpool_alloc_block_t), 
                                                                     FDB_NO_HINT);
          *new_block = (fdb_txpool_alloc_block_t){};
          new_block->p_data = fdb_pool_alloc_alloc(&palloc->m_data_palloc, 
                                                   palloc->m_data_alignment, 
                                                   palloc->m_data_size, 
                                                   FDB_NO_HINT);

          memcpy(new_block->p_data, candidate_version->p_data, palloc->m_data_size);
          new_block->m_ts = tx->m_txversion;
          if(candidate_version != ref.p_main)
          {
            FDB_ASSERT(new_block->m_ts > candidate_version->m_ts && "Invariant violation, new block ts must be larger than previous one");
            new_block->p_next_version = candidate_version;
          }
          fdb_mem_barrier(); // to make sure that the pointer in new_ref and other values are set before the pointer of base_ref is set
          ref.p_main->p_next_version = new_block; 
          fdb_mem_barrier(); // to make sure that the pointer in new_ref and other values are set before the pointer of base_ref is set
          ret = new_block->p_data;
        }
        else
        {
          ret = candidate_version->p_data;
        }
      }
      break;
  }
  return ret;
}

bool
fdb_txpool_alloc_gc(struct fdb_txpool_alloc_t* palloc, 
                    struct fdb_txthread_ctx_t* txtctx,
                    uint64_t orv,
                    fdb_txpool_alloc_block_t* root, 
                    bool force)
{

  bool gced = false;
  if(force && root != NULL)
  {
    fdb_txpool_alloc_block_t* next_block = root;
    while(next_block != NULL)
    {
      fdb_txpool_alloc_block_t* tmp = next_block->p_next_version;
      if(next_block->p_data != NULL)
      {
        fdb_pool_alloc_free(&palloc->m_data_palloc, next_block->p_data);
      }
      fdb_pool_alloc_free(&palloc->m_block_palloc, next_block);
      next_block = tmp;
    }
    gced = true;
  }
  else
  {
    // Look for versions that can be safely removed. These are those whose 
    // ts is smaller than the older version whose ts is larger than the
    // safepoint, which is the id of the youngest committed transaction such
    // that there is not an older transaction still running.
    // Find the youngest version with ts older or equal than safepoint
    fdb_txpool_alloc_block_t* next_candidate = root->p_next_version;
    while(next_candidate != NULL && 
          next_candidate->m_ts > orv)
    {
      FDB_ASSERT((next_candidate->p_next_version == NULL || 
                  (next_candidate->m_ts > next_candidate->p_next_version->m_ts)) && "Invariant violation of decreasing order of ts in chain");
      next_candidate = next_candidate->p_next_version;
    }


    // Now we can safely remove the older versions than next
    if(next_candidate != NULL)
    {
      fdb_txpool_alloc_block_t* tmp = next_candidate->p_next_version;
      next_candidate->p_next_version = NULL;
      fdb_mem_barrier(); // to make sure that next_ref pointer is set to NULL before any modification to the version
      next_candidate = tmp;
      while(next_candidate != NULL)
      {
        tmp = next_candidate->p_next_version;
        if(next_candidate->p_data != NULL)
        {
          fdb_pool_alloc_free(&palloc->m_data_palloc, next_candidate->p_data);
        }
        fdb_pool_alloc_free(&palloc->m_block_palloc, next_candidate);
        next_candidate = tmp;
      }
    }

    // Check if there is a single and committed extra version
    fdb_txpool_alloc_block_t* youngest_version = root->p_next_version;
    if(youngest_version != NULL &&
       youngest_version->p_next_version == NULL &&
       youngest_version->m_ts <= orv)
    {
      if(youngest_version->p_data != NULL)
      {
        if(root->p_data == NULL)
        {
          root->p_data = fdb_pool_alloc_alloc(&palloc->m_data_palloc, 
                                              palloc->m_data_alignment, 
                                              palloc->m_data_size, 
                                              FDB_NO_HINT);
        }
        memcpy(root->p_data, youngest_version->p_data, palloc->m_data_size);
      }
      else
      {
        fdb_pool_alloc_free(&palloc->m_data_palloc, root->p_data);
        root->p_data = NULL;
      }
      root->m_ts = youngest_version->m_ts;
      root->p_next_version = NULL;
      fdb_txthread_ctx_gc_block(txtctx, palloc, youngest_version);
      fdb_mem_barrier(); // to make sure that base_ref next_ref pointer is set to NULL before any modification to the version
    }

    if(root->p_data == NULL &&          // freed block
       root->p_next_version == NULL &&  // no more recent versions, so it is a root block from a reference that was freed
       root->m_ts < orv)                // version is older than oldest running, can be safely freed since no more recent transaction should access it
    {
      fdb_pool_alloc_free(&palloc->m_block_palloc, root);
      gced = true;
    }

  }

  return gced;
}
