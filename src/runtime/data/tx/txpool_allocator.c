

#include "txpool_allocator.h"
#include "../../../common/platform.h"
#include "string.h"


/**
 * \brief inits a pool allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new pool allocator
 */
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
  palloc->m_data_offset = sizeof(fdb_txpool_alloc_ref_t) % min_alignment; // the header can be aligned to min alingment
  if(palloc->m_data_offset != 0)
  {
    palloc->m_data_offset = sizeof(fdb_txpool_alloc_ref_t) + (min_alignment - palloc->m_data_offset);
  }
  uint32_t real_block_size = palloc->m_data_offset + block_size;

  FDB_ASSERT(real_block_size <= page_size && "Misconfigured txpool allocator. Page size is too small");

  fdb_pool_alloc_init(&palloc->m_palloc, alignment, real_block_size, page_size, allocator);

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
  fdb_mutex_lock(&palloc->m_mutex);
  fdb_pool_alloc_flush(&palloc->m_palloc);
  fdb_mutex_unlock(&palloc->m_mutex);
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

  fdb_mutex_lock(&palloc->m_mutex);

  void* ptr = fdb_pool_alloc_alloc(&palloc->m_palloc, 
                                   alignment, 
                                   size, 
                                   hint);
  fdb_txpool_alloc_ref_t* ref = (fdb_txpool_alloc_ref_t*)ptr;
  ref->m_ts = tx->m_id;
  ref->p_data = ((uint8_t*)ptr) + palloc->m_data_offset;
  ref->p_next_ref = NULL;

  fdb_mutex_unlock(&palloc->m_mutex);
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
                           fdb_txpool_alloc_ref_t* ref )
{
  fdb_mutex_lock(&palloc->m_mutex);
  fdb_mutex_unlock(&palloc->m_mutex);
}

void*
fdb_txpool_ptr(fdb_txpool_alloc_t* palloc, 
               fdb_tx_t* tx, 
               fdb_txthread_ctx_t* txtctx,
               fdb_txpool_alloc_ref_t* ref)
{
}
