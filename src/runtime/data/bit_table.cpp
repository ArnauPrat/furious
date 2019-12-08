
#include "bit_table.h"
#include "table.h"

namespace furious
{

BitTable::BitTable() :
m_size(0)
{
  m_bitmaps = (btree_t*) mem_alloc(1, sizeof(btree_t), -1);
  btree_init(m_bitmaps);
}

BitTable::~BitTable()
{
  BTIterator it(m_bitmaps);
  while(it.has_next())
  {
    btree_entry_t entry = it.next();
    bitmap_release((bt_block_t*)entry.p_value);
    mem_free(entry.p_value);
  }
  m_size = 0;

  btree_release(m_bitmaps);
  mem_free(m_bitmaps);
}

bool
BitTable::exists(entity_id_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  bt_block_t* bitmap = (bt_block_t*)btree_get(m_bitmaps, bitset_id);
  if(bitmap == nullptr) 
  {
    return false;
  }
  return bitmap_is_set(bitmap, id % TABLE_BLOCK_SIZE);
}

void
BitTable::add(entity_id_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  bt_block_t* bitmap = (bt_block_t*)btree_get(m_bitmaps, bitset_id);
  if(bitmap == nullptr) 
  {
    void** ptr = btree_insert(m_bitmaps, bitset_id).p_place;
    *ptr = mem_alloc(1, sizeof(bt_block_t), -1);
    bitmap = (bt_block_t*)*ptr;
    bitmap_init(bitmap);
  }
  FURIOUS_ASSERT(bitmap->m_num_set <= TABLE_BLOCK_SIZE && "Bitmap num set out of bounds");
  
  uint32_t offset = id % TABLE_BLOCK_SIZE;
  if(!bitmap_is_set(bitmap,offset))
  {
    ++m_size;
    bitmap_set(bitmap,offset);
  }
}

void
BitTable::remove(entity_id_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  bt_block_t* bitmap = get_bitset(bitset_id);
  if(bitmap == nullptr) 
  {
    return;
  }
  uint32_t offset = id % TABLE_BLOCK_SIZE;
  if(bitmap_is_set(bitmap,offset))
  {
    --m_size;
    bitmap_unset(bitmap,id % TABLE_BLOCK_SIZE);
  }
}

const bt_block_t* 
BitTable::get_bitmap(entity_id_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  bt_block_t* bitmap = get_bitset(bitset_id);
  if(bitmap == nullptr) 
  {
    void** ptr = btree_insert(m_bitmaps, bitset_id).p_place;
    *ptr = mem_alloc(1, sizeof(bt_block_t), -1);
    bitmap = (bt_block_t*)*ptr;
    bitmap_init(bitmap);
  }
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
  BTIterator it(m_bitmaps);
  while(it.has_next())
  {
    btree_entry_t entry = it.next();
    bitmap_nullify((bt_block_t*)entry.p_value);
  }
  m_size = 0;
}

void
BitTable::apply_bitset(uint32_t id,
                       const bt_block_t* bitmap,
                       logic_operation_t operation)
{
  bt_block_t* bm = get_bitset(id);
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
        void** ptr = btree_insert(m_bitmaps, id).p_place;
        *ptr = mem_alloc(1, sizeof(bt_block_t), -1);
        bt_block_t* nbitmap = (bt_block_t*)*ptr;
        bitmap_init(nbitmap);
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

bt_block_t*
BitTable::get_bitset(uint32_t bitset_id) const
{
  return (bt_block_t*)btree_get(m_bitmaps,bitset_id);
}

void
bittable_union(BitTable* first, const BitTable* second)
{
  BTIterator it(second->m_bitmaps);
  while(it.has_next())
  {
    btree_entry_t entry = it.next();
    first->apply_bitset(entry.m_key, 
                        (bt_block_t*)entry.p_value, 
                        BitTable::logic_operation_t::E_OR);
  }
}

void
bittable_difference(BitTable* first, const BitTable* second)
{
  BTIterator it(second->m_bitmaps);
  while(it.has_next())
  {
    btree_entry_t entry = it.next();
    first->apply_bitset(entry.m_key, 
                        (bt_block_t*)entry.p_value, 
                        BitTable::logic_operation_t::E_DIFF);
  }
}

} /* furious
*/ 
