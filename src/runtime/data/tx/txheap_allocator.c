

#include "txheap_allocator.h"
#include "../../../common/platform.h"
#include "string.h"
#include <math.h>


static uint32_t actual_size(uint32_t size)
{
  return ceil(log2(size));
}

void
fdb_txheap_alloc_init(struct fdb_txheap_alloc_t* palloc, 
                      uint32_t alignment, 
                      uint32_t page_size,
                      struct fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  *palloc = (struct fdb_txheap_alloc_t){};
  palloc->p_allocator = allocator;
  palloc->m_alignment = alignment;
  palloc->m_page_size = page_size;

  fdb_btree_factory_init(&palloc->m_btree_factory, 
                         allocator);

  fdb_btree_init(&palloc->m_pools, 
                 &palloc->m_btree_factory);


}

void
fdb_txheap_alloc_release(struct fdb_txheap_alloc_t* palloc, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx)
{
  struct fdb_btree_iter_t btree_iter;
  fdb_btree_iter_init(&btree_iter, 
                      &palloc->m_pools);
  while(fdb_btree_iter_has_next(&btree_iter))
  {
    struct fdb_txpool_alloc_t* alloc = (struct fdb_txpool_alloc_t*) fdb_btree_iter_next(&btree_iter).p_value;
    fdb_txpool_alloc_destroy(alloc, 
                             tx, 
                             txtctx);
  }
  fdb_btree_iter_release(&btree_iter);
  fdb_btree_release(&palloc->m_pools);
  fdb_btree_factory_release(&palloc->m_btree_factory);
}

void
fdb_txheap_alloc_flush(struct fdb_txheap_alloc_t* palloc)
{
  struct fdb_btree_iter_t btree_iter;
  fdb_btree_iter_init(&btree_iter, 
                      &palloc->m_pools);
  while(fdb_btree_iter_has_next(&btree_iter))
  {
    struct fdb_txpool_alloc_t* alloc = (struct fdb_txpool_alloc_t*) fdb_btree_iter_next(&btree_iter).p_value;
    fdb_txpool_alloc_flush(alloc);
  }
  fdb_btree_iter_release(&btree_iter);
}

struct fdb_txheap_alloc_ref_t 
fdb_txheap_alloc_alloc(struct fdb_txheap_alloc_t* palloc, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       uint32_t alignment, 
                       uint32_t size,
                       uint32_t hint)
{
  FDB_ASSERT(tx->m_tx_type == E_READ_WRITE && "Read transactions cannot allocate from a transactional pool");
  FDB_ASSERT(alignment == palloc->m_alignment);


  uint32_t asize = actual_size(size);
  struct fdb_txpool_alloc_t* alloc = fdb_btree_get(&palloc->m_pools, 
                                                  asize);
  if(alloc == NULL)
  {
    alloc = fdb_txpool_alloc_create(alignment, 
                                    asize,
                                    FDB_TXHEAP_PAGE_SIZE, 
                                    palloc->p_allocator);

    fdb_btree_insert(&palloc->m_pools, 
                     asize, 
                     alloc);
  }

  struct fdb_txheap_alloc_ref_t ref = {.p_alloc = alloc, 
                                       .m_ref = fdb_txpool_alloc_alloc(alloc, 
                                                                       tx, 
                                                                       txtctx, 
                                                                       palloc->m_alignment, 
                                                                       asize, 
                                                                       FDB_NO_HINT)};

  return ref;
}


void fdb_txheap_alloc_free(struct fdb_txheap_alloc_t* palloc, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           struct fdb_txheap_alloc_ref_t ref)
{
  fdb_txpool_alloc_free(ref.p_alloc, 
                        tx, 
                        txtctx,
                        ref.m_ref);

}


void*
fdb_txheap_alloc_ptr(struct fdb_txheap_alloc_t* palloc, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx,
                     struct fdb_txheap_alloc_ref_t ref, 
                     bool write)
{
  return fdb_txpool_alloc_ptr(ref.p_alloc, 
                              tx, 
                              txtctx, 
                              ref.m_ref, 
                              write);
}

