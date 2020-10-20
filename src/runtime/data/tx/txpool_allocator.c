

#include "txpool_allocator.h"
#include "../../../common/platform.h"
#include "string.h"

typedef struct fdb_txpool_alloc_block_t
{
  uint64_t                                    m_ts;
  void*                                       p_data;
  struct fdb_txpool_alloc_block_t* volatile   p_next_version;
  bool                                        m_freed;
} fdb_txpool_alloc_block_t;



void
fdb_txpool_alloc_init(fdb_txpool_alloc_t* palloc, 
                      uint32_t alignment, 
                      uint32_t block_size, 
                      uint32_t page_size,
                      fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  memset(palloc, 0, sizeof(fdb_txpool_alloc_t));
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

void
fdb_txpool_alloc_release(fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_release(&palloc->m_data_palloc);
  fdb_pool_alloc_release(&palloc->m_block_palloc);
}

void
fdb_txpool_alloc_flush(fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_flush(&palloc->m_data_palloc);
  fdb_pool_alloc_flush(&palloc->m_block_palloc);
}

void
fdb_txpool_alloc_ref_init(fdb_txpool_alloc_t* palloc, 
                          fdb_txpool_alloc_ref_t* ref)
{
  *ref = (fdb_txpool_alloc_ref_t){};
  ref->p_main = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                     FDB_MIN_ALIGNMENT, 
                                     sizeof(fdb_txpool_alloc_block_t), 
                                     FDB_NO_HINT);
  *ref->p_main = (fdb_txpool_alloc_block_t){};
  ref->p_main->m_freed = true;
}

void
fdb_txpool_alloc_ref_release(fdb_txpool_alloc_t* palloc, 
                             fdb_txpool_alloc_ref_t* ref)
{
  fdb_txpool_alloc_block_t* next = ref->p_next_version;
  while(next != NULL)
  {
    if(next->p_data != NULL)
    {
      fdb_pool_alloc_free(&palloc->m_data_palloc, next->p_data);
    }
    fdb_txpool_alloc_block_t* tmp = next->p_next_version;
    fdb_pool_alloc_free(&palloc->m_block_palloc, next);
    next = tmp;
  }

  if(ref->p_main->p_data != NULL)
  {
    fdb_pool_alloc_free(&palloc->m_data_palloc, ref->p_main->p_data);
  }
  fdb_pool_alloc_free(&palloc->m_block_palloc, ref->p_main);
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
void fdb_txpool_alloc_alloc(fdb_txpool_alloc_t* palloc, 
                            fdb_tx_t* tx, 
                            fdb_txthread_ctx_t* txtctx,
                            uint32_t alignment, 
                            uint32_t size,
                            uint32_t hint, 
                            fdb_txpool_alloc_ref_t* ref)
{
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "Read transactions cannot allocate from a transactional pool");
  FDB_ASSERT(size == palloc->m_data_size);
  FDB_ASSERT(alignment == palloc->m_data_alignment);
  FDB_ASSERT(((ref->p_next_version != NULL && ref->p_next_version->m_freed) ||
              (ref->p_next_version == NULL && ref->p_main->m_freed)) && "Alloc can only be called on freed or just-initialized references");


  fdb_txpool_alloc_block_t* new_block = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                                             FDB_MIN_ALIGNMENT, 
                                                             sizeof(fdb_txpool_alloc_block_t), 
                                                             FDB_NO_HINT);
  *new_block = (fdb_txpool_alloc_block_t){};
  new_block->p_data = fdb_pool_alloc_alloc(&palloc->m_data_palloc, 
                                           palloc->m_data_alignment, 
                                           palloc->m_data_size, 
                                           hint);
  new_block->p_next_version = ref->p_next_version;
  new_block->p_next_version=NULL;

  new_block->m_ts = tx->m_txversion;
  new_block->m_freed = false;
  fdb_mem_barrier();
  ref->p_next_version = new_block; 
}


/**
 * @brief Frees an allocated memory block
 *
 * #param ptr The state
 * @param ptr The pointer to the memory block to free
 */
void fdb_txpool_alloc_free(fdb_txpool_alloc_t* palloc, 
                           fdb_tx_t* tx, 
                           fdb_txthread_ctx_t* txtctx,
                           fdb_txpool_alloc_ref_t* ref )
{

  FDB_ASSERT(ref->p_main != NULL && "Detected double free");
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "A read-only tx cannot free a block");

  fdb_txpool_alloc_gc(palloc, 
                      txtctx,
                      tx->m_ortxversion,
                      ref, 
                      false);

  fdb_txpool_alloc_block_t* candidate_version = ref->p_next_version;
  while(candidate_version != NULL && 
        candidate_version->m_ts > tx->m_txversion)
  {
    candidate_version = candidate_version->p_next_version;
  }

  if(candidate_version == NULL)
  {
    candidate_version = ref->p_main;
  }

  FDB_ASSERT((candidate_version->m_ts <= tx->m_txversion && !candidate_version->m_freed) && "Detected double free");


  if(candidate_version->m_ts <= tx->m_ortxversion)
  {
    fdb_txpool_alloc_block_t* new_block = fdb_pool_alloc_alloc(&palloc->m_block_palloc, 
                                                               FDB_MIN_ALIGNMENT, 
                                                               sizeof(fdb_txpool_alloc_block_t), 
                                                               FDB_NO_HINT);
    *new_block = (fdb_txpool_alloc_block_t){};
    if(candidate_version != ref->p_main)
    {
      new_block->p_next_version = candidate_version->p_next_version;
    }
    else
    {
      new_block->p_next_version=NULL;
    }

    new_block->m_ts = tx->m_txversion;
    new_block->m_freed = true;
    fdb_mem_barrier();
    ref->p_next_version = new_block; 
  }
  else
  {
    candidate_version->m_freed = true;
  }

  fdb_txthread_ctx_add_entry(txtctx, 
                             palloc, 
                             ref);
}


void*
fdb_txpool_alloc_ptr(fdb_txpool_alloc_t* palloc, 
                     fdb_tx_t* tx, 
                     fdb_txthread_ctx_t* txtctx,
                     fdb_txpool_alloc_ref_t* ref, 
                     bool write)
{

  if(ref->p_main == NULL)
  {
    return NULL;
  }

  if(tx->m_tx_type == E_READ_WRITE)
  {
    // Garbage collect stale blocks
    fdb_txpool_alloc_gc(palloc, 
                        txtctx,
                        tx->m_ortxversion,
                        ref, 
                        false);
  }


  void* ret = NULL;
  fdb_txpool_alloc_block_t* candidate_version = ref->p_next_version;
  while(candidate_version != NULL && 
        candidate_version->m_ts > tx->m_txversion)
  {

    FDB_ASSERT(candidate_version->p_next_version == NULL || 
               (candidate_version->m_ts > candidate_version->p_next_version->m_ts && "Invariant violation of decreasing order of ts in chain" ));
    candidate_version = candidate_version->p_next_version;
  }

  if(candidate_version == NULL)
  {
    candidate_version = ref->p_main;
  }

  if(candidate_version->m_ts > tx->m_txversion ||
     candidate_version->m_freed) // This tx cannot still see this verstion 
  {
    return NULL;
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
          FDB_ASSERT((candidate_version == ref->p_main || candidate_version == ref->p_next_version) && "Invariant violation. Write transactions should always get most recent version");

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
          if(candidate_version != ref->p_main)
          {
            FDB_ASSERT(new_block->m_ts > candidate_version->m_ts && "Invariant violation, new block ts must be larger than previous one");
            new_block->p_next_version = candidate_version;
          }
          fdb_mem_barrier(); // to make sure that the pointer in new_ref and other values are set before the pointer of base_ref is set
          ref->p_next_version = new_block; 
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
fdb_txpool_alloc_gc(fdb_txpool_alloc_t* palloc, 
                    fdb_txthread_ctx_t* txtctx,
                    uint64_t orv,
                    fdb_txpool_alloc_ref_t* ref, 
                    bool force)
{

  if((ref->p_main->p_next_version != NULL) && (ref->p_main->m_ts < orv || force)) 
  {
    fdb_pool_alloc_free(&palloc->m_data_palloc, ref->p_main->p_next_version->p_data);
    fdb_pool_alloc_free(&palloc->m_block_palloc, ref->p_main->p_next_version);
    ref->p_main->p_next_version = NULL;
  }

  if(force && ref->p_next_version != NULL)
  {
    fdb_txpool_alloc_block_t* next_block = ref->p_next_version->p_next_version;
    while(next_block != NULL)
    {
      fdb_txpool_alloc_block_t* tmp = next_block->p_next_version;
      fdb_pool_alloc_free(&palloc->m_data_palloc, next_block->p_data);
      fdb_pool_alloc_free(&palloc->m_block_palloc, next_block);
      next_block = tmp;
    }
  }
  else
  {
    // Look for versions that can be safely removed. These are thos whose 
    // ts is smaller than the older version whose ts is larger than the
    // safepoint, which is the id of the youngest committed transaction such
    // that there is not an older transaction still running.
    // Find the youngest version with ts older or equal than safepoint
    fdb_txpool_alloc_block_t* next_candidate = ref->p_next_version;
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
        fdb_pool_alloc_free(&palloc->m_data_palloc, next_candidate->p_data);
        fdb_pool_alloc_free(&palloc->m_block_palloc, next_candidate);
        next_candidate = tmp;
      }
    }
  }

  // Check if there is a single and committed extra version
  fdb_txpool_alloc_block_t* youngest_version = ref->p_next_version;
  if(youngest_version != NULL &&
     youngest_version->p_next_version == NULL &&
     youngest_version->m_ts <= orv &&
     ref->p_main->p_next_version == NULL)
  {
    if(youngest_version->p_data != NULL)
    {
      if(ref->p_main->p_data == NULL)
      {
        ref->p_main->p_data = fdb_pool_alloc_alloc(&palloc->m_data_palloc, 
                                                   palloc->m_data_alignment, 
                                                   palloc->m_data_size, 
                                                   FDB_NO_HINT);
      }
      memcpy(ref->p_main->p_data, youngest_version->p_data, palloc->m_data_size);
    }
    else
    {
      FDB_ASSERT(youngest_version->m_freed);
      fdb_pool_alloc_free(&palloc->m_data_palloc, ref->p_main->p_data);
      ref->p_main->p_data = NULL;
    }
    ref->p_main->m_ts = youngest_version->m_ts;
    ref->p_next_version = NULL;
    ref->p_main->m_freed = youngest_version->m_freed;
    if(force)
    {
      if(!youngest_version->m_freed)
      {
        fdb_pool_alloc_free(&palloc->m_data_palloc, youngest_version->p_data);
      }
      fdb_pool_alloc_free(&palloc->m_block_palloc, youngest_version);
    }
    else
    {
      ref->p_main->p_next_version = youngest_version;
    }
    fdb_mem_barrier(); // to make sure that base_ref next_ref pointer is set to NULL before any modification to the version
  }

  return ref->p_next_version == NULL && ref->p_main->p_next_version == NULL;

}
