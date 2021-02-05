
#include "tx/txpool_allocator.h"
#include "txbittable.h"
#include "txbitmap_utils.h"


struct fdb_txpool_alloc_ref_t
fdb_txbittable_get_bitset(struct fdb_txbittable_t* bt, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx, 
                          uint32_t bitset_id)
{
  return fdb_txbtree_get(&bt->m_bitmaps, 
                         tx, 
                         txtctx, 
                         bitset_id);
}

void
fdb_txbittable_apply_bitset(struct fdb_txbittable_t* bt, 
                            struct fdb_tx_t* tx,
                            struct fdb_txthread_ctx_t* txtctx,
                            uint32_t id,
                            FDB_RESTRICT(struct fdb_txbitmap_t*) bitmap,
                            enum fdb_txbittable_op_t op)
{
  struct fdb_txpool_alloc_ref_t bm_ref = fdb_txbittable_get_bitset(bt, 
                                                                   tx, 
                                                                   txtctx,
                                                                   id);
  switch(op)
  {
    case E_TXBITTABLE_AND:
      if(bm_ref.p_main != NULL)
      {

        FDB_RESTRICT(struct fdb_txbitmap_t*) bm = fdb_txpool_alloc_ptr(bt->p_factory->m_txbitmap_factory.p_txbitmap_alloc, 
                                                                       tx, 
                                                                       txtctx, 
                                                                       bm_ref, 
                                                                       false);
        fdb_txbitmap_set_and(bm, 
                             bitmap, 
                             tx, 
                             txtctx);
      }
      break;
    case E_TXBITTABLE_OR:
      if(bm_ref.p_main != NULL)
      {
        FDB_RESTRICT(struct fdb_txbitmap_t*) bm = fdb_txpool_alloc_ptr(bt->p_factory->m_txbitmap_factory.p_txbitmap_alloc, 
                                                                       tx, 
                                                                       txtctx, 
                                                                       bm_ref, 
                                                                       false);
        fdb_txbitmap_set_or(bm, 
                            bitmap, 
                            tx, 
                            txtctx);
      }
      else
      {
        struct fdb_txpool_alloc_ref_t nbitmap_ref = fdb_txpool_alloc_alloc(bt->p_factory->p_txbitmap_alloc, 
                                                                           tx, 
                                                                           txtctx,
                                                                           FDB_TXBITTABLE_BITMAP_ALIGNMENT, 
                                                                           sizeof(struct fdb_txbitmap_t), 
                                                                           FDB_NO_HINT);

        FDB_RESTRICT(struct fdb_txbitmap_t*) nbitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                                            tx, 
                                                                            txtctx, 
                                                                            nbitmap_ref, 
                                                                            false);
        fdb_txbitmap_init(nbitmap, 
                          &bt->p_factory->m_txbitmap_factory, 
                          tx, 
                          txtctx);
        fdb_txbtree_insert(&bt->m_bitmaps, 
                           tx, 
                           txtctx, 
                           id, 
                           nbitmap_ref);
        fdb_txbitmap_set_txbitmap(nbitmap,
                                  bitmap, 
                                  tx, 
                                  txtctx);
      }
      break;
    case E_TXBITTABLE_DIFF:
      if(bm_ref.p_main != NULL)
      {
        FDB_RESTRICT(struct fdb_txbitmap_t*) bm = fdb_txpool_alloc_ptr(bt->p_factory->m_txbitmap_factory.p_txbitmap_alloc, 
                                                                       tx, 
                                                                       txtctx, 
                                                                       bm_ref, 
                                                                       false);
        fdb_txbitmap_set_diff(bm, 
                              bitmap, 
                              tx, 
                              txtctx);
      }
      break;
  };
}

void
fdb_txbittable_factory_init(struct fdb_txbittable_factory_t* factory, 
                            struct fdb_mem_allocator_t* allocator)
{
  *factory = (struct fdb_txbittable_factory_t){};

  factory->p_txbitmap_alloc = fdb_txpool_alloc_create(FDB_TXBITTABLE_BITMAP_ALIGNMENT,
                                                      sizeof(struct fdb_txbitmap_t), 
                                                      FDB_TXBITTABLE_BITMAP_PAGE_SIZE, 
                                                      allocator);


  fdb_txbitmap_factory_init(&factory->m_txbitmap_factory, 
                            allocator,
                            FDB_TXTABLE_BLOCK_SIZE);

  fdb_txbtree_factory_init(&factory->m_btree_factory, 
                           allocator);
}

void
fdb_txbittable_factory_release(struct fdb_txbittable_factory_t* factory, 
                               struct fdb_tx_t* tx, 
                               struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txbitmap_factory_release(&factory->m_txbitmap_factory, 
                               tx, 
                               txtctx);
  fdb_txbtree_factory_release(&factory->m_btree_factory, 
                              tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(factory->p_txbitmap_alloc, 
                           tx, 
                           txtctx);
}


void
fdb_txbittable_init(struct fdb_txbittable_t* bt, 
                    struct fdb_txbittable_factory_t* factory, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx) 
{
  *bt = (struct fdb_txbittable_t){};
  bt->p_factory = factory;
  fdb_txbtree_init(&bt->m_bitmaps, 
                   &factory->m_btree_factory, 
                   tx, 
                   txtctx);
}

void
fdb_txbittable_release(struct fdb_txbittable_t* bt, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx)
{
  struct fdb_txbtree_iter_t it;
  fdb_txbtree_iter_init(&it, 
                        &bt->m_bitmaps, 
                        tx, 
                        txtctx);
  while(fdb_txbtree_iter_has_next(&it))
  {
    struct fdb_txbtree_entry_t entry = fdb_txbtree_iter_next(&it);
    struct fdb_txbitmap_t* txbitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc,
                                                           tx, 
                                                           txtctx, 
                                                           entry.m_value_ref, 
                                                           true); 
    fdb_txbitmap_release(txbitmap, 
                         tx, 
                         txtctx);
    fdb_txpool_alloc_free(bt->p_factory->p_txbitmap_alloc, 
                          tx, 
                          txtctx,
                          entry.m_value_ref);
  }
  fdb_txbtree_iter_release(&it);
  fdb_txbtree_release(&bt->m_bitmaps, 
                      tx, 
                      txtctx);
}

bool
fdb_txbittable_exists(struct fdb_txbittable_t* bt, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      entity_id_t id) 
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_txpool_alloc_ref_t ref = fdb_txbtree_get(&bt->m_bitmaps, 
                                                      tx, 
                                                      txtctx, 
                                                      bitset_id);
  if(ref.p_main == NULL) 
  {
    return false;
  }

  struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                       tx, 
                                                       txtctx, 
                                                       ref, 
                                                       false);
  return fdb_txbitmap_is_set(bitmap, 
                             tx, 
                             txtctx,
                             id % FDB_TXTABLE_BLOCK_SIZE);
}

void
fdb_txbittable_add(struct fdb_txbittable_t* bt, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx,
                   entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_txpool_alloc_ref_t bitmap_ref = fdb_txbtree_get(&bt->m_bitmaps, 
                                                             tx, 
                                                             txtctx, 
                                                             bitset_id);
  struct fdb_txbitmap_t* bitmap = NULL;
  if(bitmap_ref.p_main == NULL) 
  {
    bitmap_ref = fdb_txpool_alloc_alloc(bt->p_factory->p_txbitmap_alloc, 
                                        tx, 
                                        txtctx, 
                                        FDB_TXBITTABLE_BITMAP_ALIGNMENT, 
                                        sizeof(struct fdb_txbitmap_t), 
                                        FDB_NO_HINT);
    bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                         tx, 
                                                         txtctx, 
                                                         bitmap_ref, 
                                                         true);

    fdb_txbitmap_init(bitmap, 
                      &bt->p_factory->m_txbitmap_factory, 
                      tx, 
                      txtctx);
    fdb_txbtree_insert(&bt->m_bitmaps,
                       tx, 
                       txtctx, 
                       bitset_id, 
                       bitmap_ref);
  }
  else
  {
    bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                         tx, 
                                                         txtctx, 
                                                         bitmap_ref, 
                                                         false);

  }
  uint32_t offset = id % FDB_TXTABLE_BLOCK_SIZE;
  if(!fdb_txbitmap_is_set(bitmap,
                          tx, 
                          txtctx, 
                          offset))
  {
    fdb_txbitmap_set(bitmap,
                   tx, 
                   txtctx, 
                   offset);
  }
}

void
fdb_txbittable_remove(struct fdb_txbittable_t* bt, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, 
                      entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_txpool_alloc_ref_t bitmap_ref = fdb_txbittable_get_bitset(bt, 
                                                                       tx, 
                                                                       txtctx, 
                                                                       bitset_id);
  if(bitmap_ref.p_main == NULL) 
  {
    return;
  }

  struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                       tx, 
                                                       txtctx, 
                                                       bitmap_ref, 
                                                       false);

  uint32_t offset = id % FDB_TXTABLE_BLOCK_SIZE;
  if(fdb_txbitmap_is_set(bitmap, tx, txtctx, offset))
  {
    fdb_txbitmap_unset(bitmap,tx, txtctx, id % FDB_TXTABLE_BLOCK_SIZE);
  }
}

struct fdb_txbitmap_t* 
fdb_txbittable_get_bitmap(struct fdb_txbittable_t* bt, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_txpool_alloc_ref_t bitmap_ref = fdb_txbittable_get_bitset(bt, 
                                                            tx, 
                                                            txtctx, 
                                                            bitset_id);

  struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                       tx, 
                                                       txtctx, 
                                                       bitmap_ref, 
                                                       false);
  return bitmap;
}

void
fdb_txbittable_clear(struct fdb_txbittable_t* bt, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx)
{
  struct fdb_txbtree_iter_t it;
  fdb_txbtree_iter_init(&it, 
                        &bt->m_bitmaps, 
                        tx, 
                        txtctx);
  while(fdb_txbtree_iter_has_next(&it))
  {
    struct fdb_txbtree_entry_t entry = fdb_txbtree_iter_next(&it);
    struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(bt->p_factory->p_txbitmap_alloc, 
                                                         tx, 
                                                         txtctx, 
                                                         entry.m_value_ref, 
                                                         false);
    fdb_txbitmap_nullify(bitmap, 
                         tx, 
                         txtctx);
  }
  fdb_txbtree_iter_release(&it);
}



void
fdb_txbittable_union(FDB_RESTRICT(struct fdb_txbittable_t*) fst, 
                     FDB_RESTRICT(struct fdb_txbittable_t*) snd, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT( fst != snd && "Cannot call union the same BitTable");
  struct fdb_txbtree_iter_t it;
  fdb_txbtree_iter_init(&it, 
                        &snd->m_bitmaps, 
                        tx, 
                        txtctx);
  while(fdb_txbtree_iter_has_next(&it))
  {
    struct fdb_txbtree_entry_t entry = fdb_txbtree_iter_next(&it);
    struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(fst->p_factory->p_txbitmap_alloc, 
                                                         tx, 
                                                         txtctx, 
                                                         entry.m_value_ref, 
                                                         false);
    fdb_txbittable_apply_bitset(fst, 
                                tx, 
                                txtctx,
                                entry.m_key, 
                                bitmap, 
                                E_TXBITTABLE_OR);
  }
  fdb_txbtree_iter_release(&it);
}

void
fdb_txbittablE_TXBITTABLE_DIFFerence(FDB_RESTRICT(struct fdb_txbittable_t*) fst, 
                          FDB_RESTRICT(struct fdb_txbittable_t*) snd, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT( fst != snd && "Cannot call difference the same BitTable");
  struct fdb_txbtree_iter_t it;
  fdb_txbtree_iter_init(&it,
                        &snd->m_bitmaps, 
                        tx, 
                        txtctx);
  while(fdb_txbtree_iter_has_next(&it))
  {
    struct fdb_txbtree_entry_t entry = fdb_txbtree_iter_next(&it);
    struct fdb_txbitmap_t* bitmap = fdb_txpool_alloc_ptr(fst->p_factory->p_txbitmap_alloc, 
                                                         tx, 
                                                         txtctx, 
                                                         entry.m_value_ref, 
                                                         false);
    fdb_txbittable_apply_bitset(fst, 
                                tx, 
                                txtctx,
                                entry.m_key, 
                                bitmap, 
                                E_TXBITTABLE_DIFF);
  }
  fdb_txbtree_iter_release(&it);
}

