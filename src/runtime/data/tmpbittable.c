
#include "../../common/memory/pool_allocator.h"
#include "tmpbittable.h"


struct fdb_bitmap_t*
fdb_tmpbittable_get_bitset(const struct fdb_tmpbittable_t* bt, 
                        uint32_t bitset_id)
{
  return (struct fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps,bitset_id);
}

void
fdb_tmpbittable_apply_bitset(struct fdb_tmpbittable_t* bt, 
                          uint32_t id,
                          FDB_RESTRICT(const struct fdb_bitmap_t*) bitmap,
                          enum fdb_tmpbittable_op_t op)
{
  FDB_RESTRICT(struct fdb_bitmap_t*) bm = fdb_tmpbittable_get_bitset(bt, id);
  switch(op)
  {
    case E_TMPBITTABLE_AND:
      if(bm != NULL)
      {
        bt->m_size -= bm->m_num_set;
        fdb_bitmap_set_and(bm, bitmap);
        bt->m_size += bm->m_num_set;
      }
      break;
    case E_TMPBITTABLE_OR:
      if(bm != NULL)
      {
        bt->m_size -= bm->m_num_set;
        fdb_bitmap_set_or(bm, bitmap);
        bt->m_size += bm->m_num_set;
      }
      else
      {
        FDB_RESTRICT(struct fdb_bitmap_t*) nbitmap = (struct fdb_bitmap_t*)fdb_pool_alloc_alloc(&bt->p_factory->m_bitmap_alloc, 
                                                                                                FDB_BITMAP_ALIGNMENT, 
                                                                                                sizeof(struct fdb_bitmap_t), 
                                                                                                FDB_NO_HINT);
        fdb_bitmap_init(nbitmap, &bt->p_factory->m_bitmap_factory);
        fdb_btree_insert(&bt->m_bitmaps, id, nbitmap);
        fdb_bitmap_set_or(nbitmap,bitmap);
        bt->m_size += nbitmap->m_num_set;
      }
      break;
    case E_TMPBITTABLE_DIFF:
      if(bm != NULL)
      {
        bt->m_size -= bm->m_num_set;
        fdb_bitmap_set_diff(bm, bitmap);
        bt->m_size += bm->m_num_set;
      }
      break;
  };
}

void
fdb_tmpbittable_factory_init(struct fdb_tmpbittable_factory_t* factory, 
                             struct fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  *factory = (struct fdb_tmpbittable_factory_t){};
  fdb_btree_factory_init(&factory->m_btree_factory, 
                         allocator);
  fdb_bitmap_factory_init(&factory->m_bitmap_factory, 
                          FDB_TXTABLE_BLOCK_SIZE, 
                          allocator);
  fdb_pool_alloc_init(&factory->m_bitmap_alloc, 
                      FDB_TMPBITTABLE_BITMAP_ALIGNMENT, 
                      sizeof(struct fdb_bitmap_t), 
                      FDB_TMPBITTABLE_BITMAP_PAGE_SIZE, 
                      allocator);
}

void
fdb_tmpbittable_factory_release(struct fdb_tmpbittable_factory_t* factory)
{
  *factory = (struct fdb_tmpbittable_factory_t){};
  fdb_btree_factory_release(&factory->m_btree_factory);
  fdb_bitmap_factory_release(&factory->m_bitmap_factory);
  fdb_pool_alloc_release(&factory->m_bitmap_alloc);
}


void
fdb_tmpbittable_init(struct fdb_tmpbittable_t* bt, 
                     struct fdb_tmpbittable_factory_t* factory) 
{
  bt->p_factory = factory;
  bt->m_size = 0;
  fdb_btree_init(&bt->m_bitmaps, 
                 &bt->p_factory->m_btree_factory);
}

void
fdb_tmpbittable_release(struct fdb_tmpbittable_t* bt)
{
  struct fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &bt->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bitmap_release((struct fdb_bitmap_t*)entry.p_value);
    fdb_pool_alloc_free(&bt->p_factory->m_bitmap_alloc, 
             entry.p_value);
  }
  fdb_btree_iter_release(&it);
  bt->m_size = 0;
  fdb_btree_release(&bt->m_bitmaps);
}

bool
fdb_tmpbittable_exists(const struct fdb_tmpbittable_t* bt, 
                    entity_id_t id) 
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_bitmap_t* bitmap = (struct fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps, bitset_id);
  if(bitmap == NULL) 
  {
    return false;
  }
  return fdb_bitmap_is_set(bitmap, id % FDB_TXTABLE_BLOCK_SIZE);
}

void
fdb_tmpbittable_add(struct fdb_tmpbittable_t* bt, 
                 entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_bitmap_t* bitmap = (struct fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps, bitset_id);
  if(bitmap == NULL) 
  {
    bitmap = (struct fdb_bitmap_t*)fdb_pool_alloc_alloc(&bt->p_factory->m_bitmap_alloc, 
                                      FDB_BITMAP_ALIGNMENT, 
                                      sizeof(struct fdb_bitmap_t), 
                                      FDB_NO_HINT);

    fdb_bitmap_init(bitmap, &bt->p_factory->m_bitmap_factory);
    fdb_btree_insert(&bt->m_bitmaps, bitset_id, bitmap);
  }
  FDB_ASSERT(bitmap->m_num_set <= FDB_TXTABLE_BLOCK_SIZE 
             && "Bitmap num set out of bounds");

  uint32_t offset = id % FDB_TXTABLE_BLOCK_SIZE;
  if(!fdb_bitmap_is_set(bitmap,offset))
  {
    ++bt->m_size;
    fdb_bitmap_set(bitmap,offset);
  }
}

void
fdb_tmpbittable_remove(struct fdb_tmpbittable_t* bt, 
                    entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_bitmap_t* bitmap = fdb_tmpbittable_get_bitset(bt, bitset_id);
  if(bitmap == NULL) 
  {
    return;
  }
  uint32_t offset = id % FDB_TXTABLE_BLOCK_SIZE;
  if(fdb_bitmap_is_set(bitmap,offset))
  {
    bt->m_size--;
    fdb_bitmap_unset(bitmap,id % FDB_TXTABLE_BLOCK_SIZE);
  }
}

struct fdb_bitmap_t* 
fdb_tmpbittable_get_bitmap(struct fdb_tmpbittable_t* bt, 
                           entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TXTABLE_BLOCK_SIZE;
  struct fdb_bitmap_t* bitmap = fdb_tmpbittable_get_bitset(bt, bitset_id);
  return bitmap;
}

uint32_t
fdb_bittable_size(const struct fdb_tmpbittable_t* bt)
{
  return bt->m_size;
}

void
fdb_tmpbittable_clear(struct fdb_tmpbittable_t* bt)
{
  struct fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &bt->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bitmap_nullify((struct fdb_bitmap_t*)entry.p_value);
  }
  fdb_btree_iter_release(&it);
  bt->m_size = 0;
}



void
fdb_tmpbittable_union(FDB_RESTRICT(struct fdb_tmpbittable_t*) fst, 
                   FDB_RESTRICT(const struct fdb_tmpbittable_t*) snd)
{
  FDB_ASSERT( fst != snd && "Cannot call union the same BitTable");
  struct fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &snd->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_tmpbittable_apply_bitset(fst, 
                              entry.m_key, 
                              (struct fdb_bitmap_t*)entry.p_value, 
                              E_TMPBITTABLE_OR);
  }
  fdb_btree_iter_release(&it);
}

void
fdb_tmpbittable_difference(FDB_RESTRICT(struct fdb_tmpbittable_t*) fst, FDB_RESTRICT(const struct fdb_tmpbittable_t*) snd)
{
  FDB_ASSERT( fst != snd && "Cannot call difference the same BitTable");
  struct fdb_btree_iter_t it;
  fdb_btree_iter_init(&it,&snd->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_tmpbittable_apply_bitset(fst, 
                              entry.m_key, 
                              (struct fdb_bitmap_t*)entry.p_value, 
                              E_TMPBITTABLE_DIFF);
  }
  fdb_btree_iter_release(&it);
}

