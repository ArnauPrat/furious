
#include "../../common/memory/pool_allocator.h"
#include "bittable.h"


fdb_bitmap_t*
fdb_bittable_get_bitset(const fdb_bittable_t* bt, 
                        uint32_t bitset_id)
{
  return (fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps,bitset_id);
}

void
fdb_bittable_apply_bitset(fdb_bittable_t* bt, 
                          uint32_t id,
                          FDB_RESTRICT(const fdb_bitmap_t*) bitmap,
                          fdb_bittable_op_t op)
{
  FDB_RESTRICT(fdb_bitmap_t*) bm = fdb_bittable_get_bitset(bt, id);
  switch(op)
  {
    case E_AND:
      if(bm != NULL)
      {
        bt->m_size -= bm->m_num_set;
        fdb_bitmap_set_and(bm, bitmap);
        bt->m_size += bm->m_num_set;
      }
      break;
    case E_OR:
      if(bm != NULL)
      {
        bt->m_size -= bm->m_num_set;
        fdb_bitmap_set_or(bm, bitmap);
        bt->m_size += bm->m_num_set;
      }
      else
      {
        FDB_RESTRICT(fdb_bitmap_t*) nbitmap = (fdb_bitmap_t*)fdb_pool_alloc_alloc(&bt->m_bitmap_allocator, 
                                                                                  FDB_BITMAP_ALIGNMENT, 
                                                                                  sizeof(fdb_bitmap_t), 
                                                                                  FDB_NO_HINT);
        fdb_bitmap_init(nbitmap, FDB_TABLE_BLOCK_SIZE, &bt->m_bitmap_data_allocator.m_super);
        fdb_btree_insert(&bt->m_bitmaps, id, nbitmap);
        fdb_bitmap_set_or(nbitmap,bitmap);
        bt->m_size += nbitmap->m_num_set;
      }
      break;
    case E_DIFF:
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
fdb_bittable_init(fdb_bittable_t* bt, 
                  fdb_mem_allocator_t* allocator) 
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  bt->m_size = 0;
  fdb_btree_init(&bt->m_bitmaps, allocator);

  fdb_pool_alloc_init(&bt->m_bitmap_allocator, 
                      FDB_BITMAP_ALIGNMENT, 
                      sizeof(fdb_bitmap_t), 
                      FDB_BITMAP_PAGE_SIZE, 
                      allocator);

  fdb_pool_alloc_init(&bt->m_bitmap_data_allocator, 
                      FDB_BITMAP_ALIGNMENT, 
                      FDB_BITMAP_NUM_CHUNKS(FDB_TABLE_BLOCK_SIZE), 
                      FDB_BITMAP_DATA_PAGE_SIZE, 
                      allocator);
}

void
fdb_bittable_release(fdb_bittable_t* bt)
{
  fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &bt->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bitmap_release((fdb_bitmap_t*)entry.p_value, 
                       &bt->m_bitmap_data_allocator.m_super);
    mem_free(&bt->m_bitmap_allocator.m_super, 
             entry.p_value);
  }
  fdb_btree_iter_release(&it);
  bt->m_size = 0;
  fdb_btree_release(&bt->m_bitmaps);
  fdb_pool_alloc_release(&bt->m_bitmap_data_allocator);
  fdb_pool_alloc_release(&bt->m_bitmap_allocator);
}

bool
fdb_bittable_exists(const fdb_bittable_t* bt, 
                    entity_id_t id) 
{
  uint32_t bitset_id = id / FDB_TABLE_BLOCK_SIZE;
  fdb_bitmap_t* bitmap = (fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps, bitset_id);
  if(bitmap == NULL) 
  {
    return false;
  }
  return fdb_bitmap_is_set(bitmap, id % FDB_TABLE_BLOCK_SIZE);
}

void
fdb_bittable_add(fdb_bittable_t* bt, 
                 entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TABLE_BLOCK_SIZE;
  fdb_bitmap_t* bitmap = (fdb_bitmap_t*)fdb_btree_get(&bt->m_bitmaps, bitset_id);
  if(bitmap == NULL) 
  {
    bitmap = (fdb_bitmap_t*)mem_alloc(&bt->m_bitmap_allocator.m_super, 
                                      FDB_BITMAP_ALIGNMENT, 
                                      sizeof(fdb_bitmap_t), 
                                      FDB_NO_HINT);

    fdb_bitmap_init(bitmap, FDB_TABLE_BLOCK_SIZE, &bt->m_bitmap_data_allocator.m_super);
    fdb_btree_insert(&bt->m_bitmaps, bitset_id, bitmap);
  }
  FDB_ASSERT(bitmap->m_num_set <= FDB_TABLE_BLOCK_SIZE 
             && "Bitmap num set out of bounds");

  uint32_t offset = id % FDB_TABLE_BLOCK_SIZE;
  if(!fdb_bitmap_is_set(bitmap,offset))
  {
    ++bt->m_size;
    fdb_bitmap_set(bitmap,offset);
  }
}

void
fdb_bittable_remove(fdb_bittable_t* bt, 
                    entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TABLE_BLOCK_SIZE;
  fdb_bitmap_t* bitmap = fdb_bittable_get_bitset(bt, bitset_id);
  if(bitmap == NULL) 
  {
    return;
  }
  uint32_t offset = id % FDB_TABLE_BLOCK_SIZE;
  if(fdb_bitmap_is_set(bitmap,offset))
  {
    bt->m_size--;
    fdb_bitmap_unset(bitmap,id % FDB_TABLE_BLOCK_SIZE);
  }
}

const fdb_bitmap_t* 
fdb_bittable_get_bitmap(const fdb_bittable_t* bt, 
                        entity_id_t id)
{
  uint32_t bitset_id = id / FDB_TABLE_BLOCK_SIZE;
  fdb_bitmap_t* bitmap = fdb_bittable_get_bitset(bt, bitset_id);
  return bitmap;
}

uint32_t
fdb_bittable_size(const fdb_bittable_t* bt)
{
  return bt->m_size;
}

void
fdb_bittable_clear(fdb_bittable_t* bt)
{
  fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &bt->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bitmap_nullify((fdb_bitmap_t*)entry.p_value);
  }
  fdb_btree_iter_release(&it);
  bt->m_size = 0;
}



void
fdb_bittable_union(FDB_RESTRICT(fdb_bittable_t*) fst, 
                   FDB_RESTRICT(const fdb_bittable_t*) snd)
{
  FDB_ASSERT( fst != snd && "Cannot call union the same BitTable");
  fdb_btree_iter_t it;
  fdb_btree_iter_init(&it, &snd->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bittable_apply_bitset(fst, 
                              entry.m_key, 
                              (fdb_bitmap_t*)entry.p_value, 
                              E_OR);
  }
  fdb_btree_iter_release(&it);
}

void
fdb_bittable_difference(FDB_RESTRICT(fdb_bittable_t*) fst, FDB_RESTRICT(const fdb_bittable_t*) snd)
{
  FDB_ASSERT( fst != snd && "Cannot call difference the same BitTable");
  fdb_btree_iter_t it;
  fdb_btree_iter_init(&it,&snd->m_bitmaps);
  while(fdb_btree_iter_has_next(&it))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it);
    fdb_bittable_apply_bitset(fst, 
                              entry.m_key, 
                              (fdb_bitmap_t*)entry.p_value, 
                              E_DIFF);
  }
  fdb_btree_iter_release(&it);
}

