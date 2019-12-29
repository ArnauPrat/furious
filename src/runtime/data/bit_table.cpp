
#include "bit_table.h"
#include "table.h"

namespace furious
{

BitTable::BitTable(mem_allocator_t* allocator) :
m_size(0)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")
  if(allocator != nullptr)
  {
    m_allocator = *allocator; 
  }
  else
  {
    m_allocator = global_mem_allocator;
  }

  m_bitmaps = (btree_t*) mem_alloc(&m_allocator, 
                                   1, 
                                   sizeof(btree_t), 
                                   FURIOUS_NO_HINT);
  *m_bitmaps = btree_create(&m_allocator);
}

BitTable::~BitTable()
{
  btree_iter_t it = btree_iter_create(m_bitmaps);
  while(btree_iter_has_next(&it))
  {
    btree_entry_t entry = btree_iter_next(&it);
    bitmap_destroy((bitmap_t*)entry.p_value, &m_allocator);
    mem_free(&m_allocator, 
             entry.p_value);
  }
  btree_iter_destroy(&it);
  m_size = 0;

  btree_destroy(m_bitmaps);
  mem_free(&m_allocator, 
           m_bitmaps);
}

bool
BitTable::exists(entity_id_t id) const
{
  uint32_t bitset_id = id / FURIOUS_TABLE_BLOCK_SIZE;
  bitmap_t* bitmap = (bitmap_t*)btree_get(m_bitmaps, bitset_id);
  if(bitmap == nullptr) 
  {
    return false;
  }
  return bitmap_is_set(bitmap, id % FURIOUS_TABLE_BLOCK_SIZE);
}

void
BitTable::add(entity_id_t id)
{
  uint32_t bitset_id = id / FURIOUS_TABLE_BLOCK_SIZE;
  bitmap_t* bitmap = (bitmap_t*)btree_get(m_bitmaps, bitset_id);
  if(bitmap == nullptr) 
  {
    bitmap = (bitmap_t*)mem_alloc(&m_allocator, 
                                 1, 
                                 sizeof(bitmap_t), 
                                 FURIOUS_NO_HINT);
    *bitmap = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, &m_allocator);
    btree_insert(m_bitmaps, bitset_id, bitmap);
  }
  FURIOUS_ASSERT(bitmap->m_num_set <= FURIOUS_TABLE_BLOCK_SIZE && "Bitmap num set out of bounds");
  
  uint32_t offset = id % FURIOUS_TABLE_BLOCK_SIZE;
  if(!bitmap_is_set(bitmap,offset))
  {
    ++m_size;
    bitmap_set(bitmap,offset);
  }
}

void
BitTable::remove(entity_id_t id)
{
  uint32_t bitset_id = id / FURIOUS_TABLE_BLOCK_SIZE;
  bitmap_t* bitmap = get_bitset(bitset_id);
  if(bitmap == nullptr) 
  {
    return;
  }
  uint32_t offset = id % FURIOUS_TABLE_BLOCK_SIZE;
  if(bitmap_is_set(bitmap,offset))
  {
    --m_size;
    bitmap_unset(bitmap,id % FURIOUS_TABLE_BLOCK_SIZE);
  }
}

const bitmap_t* 
BitTable::get_bitmap(entity_id_t id) const
{
  uint32_t bitset_id = id / FURIOUS_TABLE_BLOCK_SIZE;
  bitmap_t* bitmap = get_bitset(bitset_id);
  return bitmap;
}

uint32_t
BitTable::size()
{
  return m_size;
}

void
BitTable::clear()
{
  btree_iter_t it = btree_iter_create(m_bitmaps);
  while(btree_iter_has_next(&it))
  {
    btree_entry_t entry = btree_iter_next(&it);
    bitmap_nullify((bitmap_t*)entry.p_value);
  }
  btree_iter_destroy(&it);
  m_size = 0;
}

void
BitTable::apply_bitset(uint32_t id,
                       FURIOUS_RESTRICT(const bitmap_t*) bitmap,
                       logic_operation_t operation)
{
  FURIOUS_RESTRICT(bitmap_t*) bm = get_bitset(id);
  switch(operation)
  {
    case logic_operation_t::E_AND:
      if(bm != nullptr)
      {
        m_size -= bm->m_num_set;
        bitmap_set_and(bm, bitmap);
        m_size += bm->m_num_set;
      }
      break;
    case logic_operation_t::E_OR:
      if(bm != nullptr)
      {
        m_size -= bm->m_num_set;
        bitmap_set_or(bm, bitmap);
        m_size += bm->m_num_set;
      }
      else
      {
        FURIOUS_RESTRICT(bitmap_t*) nbitmap = (bitmap_t*)mem_alloc(&m_allocator, 
                                                                   1, 
                                                                   sizeof(bitmap_t), 
                                                                   -1);
        *nbitmap = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, &m_allocator);
        btree_insert(m_bitmaps, id, nbitmap);
        bitmap_set_or(nbitmap,bitmap);
        m_size += nbitmap->m_num_set;
      }
      break;
    case logic_operation_t::E_DIFF:
      if(bm != nullptr)
      {
        m_size -= bm->m_num_set;
        bitmap_set_diff(bm, bitmap);
        m_size += bm->m_num_set;
      }
      break;
  };
}

bitmap_t*
BitTable::get_bitset(uint32_t bitset_id) const
{
  return (bitmap_t*)btree_get(m_bitmaps,bitset_id);
}

void
bittable_union(FURIOUS_RESTRICT(BitTable*) first, FURIOUS_RESTRICT(const BitTable*) second)
{
  FURIOUS_ASSERT( first != second && "Cannot call union the same BitTable");
  btree_iter_t it = btree_iter_create(second->m_bitmaps);
  while(btree_iter_has_next(&it))
  {
    btree_entry_t entry = btree_iter_next(&it);
    first->apply_bitset(entry.m_key, 
                        (bitmap_t*)entry.p_value, 
                        BitTable::logic_operation_t::E_OR);
  }
  btree_iter_destroy(&it);
}

void
bittable_difference(FURIOUS_RESTRICT(BitTable*) first, FURIOUS_RESTRICT(const BitTable*) second)
{
  FURIOUS_ASSERT( first != second && "Cannot call difference the same BitTable");
  btree_iter_t it = btree_iter_create(second->m_bitmaps);
  while(btree_iter_has_next(&it))
  {
    btree_entry_t entry = btree_iter_next(&it);
    first->apply_bitset(entry.m_key, 
                        (bitmap_t*)entry.p_value, 
                        BitTable::logic_operation_t::E_DIFF);
  }
  btree_iter_destroy(&it);
}

} /* furious
*/ 
