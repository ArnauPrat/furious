

#include "txpool_allocator.h"
#include "../../../common/platform.h"
#include "string.h"

/**
 * \brief Garbage collects the stale blocks of a given reference of a tx pool
 *
 * \param palloc The txpool of the reference to garbage collect
 * \param ref The reference to garbage collect
 * \param tx The transaction executing the garbage collection  
 */
void
_fdb_txpool_alloc_gc(fdb_txpool_alloc_t* palloc, 
                     fdb_txpool_alloc_ref_t* base_ref, 
                     fdb_tx_t* tx);


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
  int32_t min_alignment = alignment > FDB_MIN_ALIGNMENT ? alignment : FDB_MIN_ALIGNMENT;
  palloc->m_data_offset = sizeof(fdb_txpool_alloc_ref_t); // the header can be aligned to min alingment
  int32_t slack = palloc->m_data_offset % min_alignment; 
  if(slack != 0)
  {
    palloc->m_data_offset += (min_alignment - slack);
  }
  uint32_t real_block_size = palloc->m_data_offset + block_size;

  FDB_ASSERT(real_block_size <= page_size && "Misconfigured txpool allocator. Page size is too small");

  palloc->m_payload_size = block_size;
  palloc->m_block_size = real_block_size;
  palloc->m_alignment = alignment;
  fdb_pool_alloc_init(&palloc->m_palloc, 
                      palloc->m_alignment, 
                      palloc->m_block_size, 
                      page_size, 
                      allocator);

  fdb_mutex_init(&palloc->m_mutex);
}

void
fdb_txpool_alloc_release(fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_release(&palloc->m_palloc);
  fdb_mutex_release(&palloc->m_mutex);
}

void
fdb_txpool_alloc_flush(fdb_txpool_alloc_t* palloc)
{
  fdb_pool_alloc_flush(&palloc->m_palloc);
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
fdb_txpool_alloc_ref_t* fdb_txpool_alloc_alloc(fdb_txpool_alloc_t* palloc, 
                                         fdb_tx_t* tx, 
                                         fdb_txthread_ctx_t* txtctx,
                                         uint32_t alignment, 
                                         uint32_t size,
                                         uint32_t hint)
{
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "Read transactions cannot allocate from a transactional pool");
  FDB_ASSERT(size == palloc->m_payload_size);
  FDB_ASSERT(alignment == palloc->m_alignment);


  void* ptr = fdb_pool_alloc_alloc(&palloc->m_palloc, 
                                   palloc->m_alignment, 
                                   palloc->m_block_size, 
                                   hint);
  fdb_txpool_alloc_ref_t* ref = (fdb_txpool_alloc_ref_t*)ptr;
  memset(ref, 0, sizeof(fdb_txpool_alloc_ref_t));
  ref->m_ts = tx->m_id;
  ref->p_data = ((uint8_t*)ptr) + palloc->m_data_offset;
  ref->p_zombie = NULL;
  ref->p_next_ref = NULL;
  ref->m_freed = false;
  ref->m_zts = 0;
  return ref;
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
                           fdb_txpool_alloc_ref_t* base_ref )
{

  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "A read-only tx cannot free a block");

  _fdb_txpool_alloc_gc(palloc, 
                       base_ref, 
                       tx);

  fdb_txpool_alloc_ref_t* candidate_version = base_ref->p_next_ref;
  while(candidate_version != NULL && 
        candidate_version->m_ts > tx->m_txversion)
  {
    candidate_version = candidate_version->p_next_ref;
  }
  if(candidate_version == NULL)
  {
    candidate_version = base_ref;
  }

  FDB_ASSERT((candidate_version->m_ts <= tx->m_txversion && !candidate_version->m_freed) && "Detected double free");


  if(candidate_version->m_ts <= tx->m_ortxversion)
  {
    fdb_txpool_alloc_ref_t* new_ref = fdb_txpool_alloc_alloc(palloc, 
                                                             tx, 
                                                             txtctx,
                                                             palloc->m_alignment, 
                                                             palloc->m_payload_size, 
                                                             FDB_NO_HINT);
    FDB_ASSERT(((((uint8_t*)new_ref)+palloc->m_data_offset+palloc->m_payload_size) - (uint8_t*)new_ref) == palloc->m_block_size && "Invalid block size. Possible error at data offset computation or pauload size");
    // No need to copy data, since this is only used to set to free
    //memcpy(new_ref->p_data, candidate_version->p_data, palloc->m_payload_size);
    memset(new_ref, 0, sizeof(fdb_txpool_alloc_ref_t));
    if(candidate_version != base_ref)
    {
      new_ref->p_next_ref = candidate_version->p_next_ref;
    }
    else
    {
      new_ref->p_next_ref=NULL;
    }

    new_ref->p_data = ((uint8_t*)new_ref) + palloc->m_data_offset;
    new_ref->m_ts = tx->m_txversion;
    new_ref->m_freed = true;
    new_ref->p_zombie = NULL;
    new_ref->m_zts = 0;
    fdb_mem_barrier();
    base_ref->p_next_ref = new_ref; 
  }
  else
  {
    candidate_version->m_freed = true;
  }

  fdb_txthread_ctx_add_entry(txtctx, palloc, base_ref, tx->m_txversion);
}

void*
fdb_txpool_alloc_ptr(fdb_txpool_alloc_t* palloc, 
               fdb_tx_t* tx, 
               fdb_txthread_ctx_t* txtctx,
               fdb_txpool_alloc_ref_t* base_ref, 
               bool write)
{
  if(tx->m_tx_type == E_READ_WRITE)
  {
    // Garbage collect stale blocks
    _fdb_txpool_alloc_gc(palloc, 
                         base_ref, 
                         tx);
  }

  void* ret = NULL;
  fdb_txpool_alloc_ref_t* candidate_version = base_ref->p_next_ref;
  while(candidate_version != NULL && 
        candidate_version->m_ts > tx->m_txversion)
  {

    FDB_ASSERT(candidate_version->p_next_ref == NULL || 
               (candidate_version->m_ts > candidate_version->p_next_ref->m_ts && "Invariant violation of decreasing order of ts in chain" ));
    candidate_version = candidate_version->p_next_ref;
  }

  if(candidate_version == NULL)
  {
    candidate_version = base_ref;
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
          FDB_ASSERT((candidate_version == base_ref || candidate_version == base_ref->p_next_ref) && "Invariant violation. Write transactions should always get most recent version");
          fdb_txpool_alloc_ref_t* new_ref = fdb_txpool_alloc_alloc(palloc, 
                                                                   tx, 
                                                                   txtctx,
                                                                   palloc->m_alignment, 
                                                                   palloc->m_payload_size, 
                                                                   FDB_NO_HINT);
          //printf("tx %lu allocates %lu\n", tx->m_id, (uint64_t)new_ref);
          fflush(stdout);
          FDB_ASSERT(((((uint8_t*)new_ref)+palloc->m_data_offset+palloc->m_payload_size) - (uint8_t*)new_ref) == palloc->m_block_size && "Invalid block size. Possible error at data offset computation or pauload size");
          memcpy(new_ref->p_data, candidate_version->p_data, palloc->m_payload_size);
          new_ref->m_ts = tx->m_txversion;
          new_ref->m_freed = false;
          new_ref->p_zombie = NULL;
          new_ref->m_zts = 0;
          if(candidate_version != base_ref)
          {
            FDB_ASSERT(new_ref->m_ts > candidate_version->m_ts && "Invariant violation, new reference ts must be larger than previous one");
            new_ref->p_next_ref = candidate_version;
          }
          fdb_mem_barrier(); // to make sure that the pointer in new_ref and other values are set before the pointer of base_ref is set
          base_ref->p_next_ref = new_ref; 
          fdb_mem_barrier(); // to make sure that the pointer in new_ref and other values are set before the pointer of base_ref is set
          ret = new_ref->p_data;
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

void
_fdb_txpool_alloc_gc(fdb_txpool_alloc_t* palloc, 
                     fdb_txpool_alloc_ref_t* base_ref, 
                     fdb_tx_t* tx)
{
  // Trying to remove existing zombies
  /*fdb_txpool_alloc_ref_t* zombie_parent = base_ref;
    fdb_txpool_alloc_ref_t* next_zombie = base_ref->p_zombie;
    while(next_zombie != NULL)
    {
    if(next_zombie->m_zts < tx->m_ortxversion)
    {
    zombie_parent->p_zombie = next_zombie->p_zombie;
    fdb_pool_alloc_free(&palloc->m_palloc, next_zombie);
    next_zombie = zombie_parent->p_zombie;
    }
    else
    {
    zombie_parent = next_zombie;
    next_zombie = next_zombie->p_zombie;
    }
    }
    */

  // Look for versions that can be safely removed. These are thos whose 
  // ts is smaller than the older version whose ts is larger than the
  // safepoint, which is the id of the youngest committed transaction such
  // that there is not an older transaction still running.
  // Find the youngest version with ts older or equal than safepoint
  fdb_txpool_alloc_ref_t* next_candidate = base_ref->p_next_ref;
  while(next_candidate != NULL && 
        next_candidate->m_ts > tx->m_ortxversion)
  {
    FDB_ASSERT((next_candidate->p_next_ref == NULL || 
               (next_candidate->m_ts > next_candidate->p_next_ref->m_ts)) && "Invariant violation of decreasing order of ts in chain");
    next_candidate = next_candidate->p_next_ref;
  }


  // Now we can safely remove the older versions than next
  /*if(next_candidate != NULL)
    {
    fdb_txpool_alloc_ref_t* tmp = next_candidate->p_next_ref;
    next_candidate->p_next_ref = NULL;
    fdb_mem_barrier(); // to make sure that next_ref pointer is set to NULL before any modification to the version
    next_candidate = tmp;
    while(next_candidate != NULL)
    {
    tmp = next_candidate->p_next_ref;
    fdb_pool_alloc_free(&palloc->m_palloc, next_candidate);
    next_candidate = tmp;
    }
    }*/



  // Check if there is a single and committed extra version
  /*
     fdb_txpool_alloc_ref_t* youngest_version = base_ref->p_next_ref;
     if(youngest_version != NULL &&
     youngest_version->p_next_ref == NULL &&
     youngest_version->m_ts <= tx->m_ortxversion)
     {
     memcpy(base_ref->p_data, youngest_version->p_data, palloc->m_payload_size);
     base_ref->m_ts = youngest_version->m_ts;
     base_ref->p_next_ref = NULL;
     fdb_mem_barrier(); // to make sure that base_ref next_ref pointer is set to NULL before any modification to the version
     youngest_version->p_zombie = base_ref->p_zombie;
     youngest_version->m_zts = tx->m_id;
     base_ref->p_zombie = youngest_version;
     }
     */

}

bool
fdb_txpool_alloc_gc_free(fdb_txpool_alloc_t* palloc, 
                         fdb_txpool_alloc_ref_t* ref, 
                         uint64_t tv, 
                         uint64_t orv)
{
  if(tv <= orv)
  {
    fdb_txpool_alloc_ref_t* next = ref->p_next_ref;
    while(next != NULL)
    {
      fdb_txpool_alloc_ref_t* tmp = next->p_next_ref;
      fdb_pool_alloc_free(&palloc->m_palloc, 
                          next);
      next = tmp;
    }

    next = ref->p_zombie;
    while(next != NULL)
    {
      fdb_txpool_alloc_ref_t* tmp = next->p_zombie;
      fdb_pool_alloc_free(&palloc->m_palloc, 
                          next);
      next = tmp;
    }

    fdb_pool_alloc_free(&palloc->m_palloc, 
                        ref);
    return true;
  }

  return false;
}
