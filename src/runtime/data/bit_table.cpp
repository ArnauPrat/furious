
#include "bit_table.h"
#include "table.h"

#include <assert.h>

namespace furious
{

BitTable::BitTable() :
m_size(0)
{
}

BitTable::~BitTable()
{
}

bool
BitTable::exists(entity_id_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    return false;
  }
  return bitmap->is_set(id % TABLE_BLOCK_SIZE);
}

void
BitTable::add(entity_id_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    bitmap = m_bitmaps.insert_new(bitset_id,TABLE_BLOCK_SIZE);
  }

  uint32_t offset = id % TABLE_BLOCK_SIZE;
  if(!bitmap->is_set(offset))
  {
    ++m_size;
    bitmap->set(offset);
  }
}

void
BitTable::remove(entity_id_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    return;
  }
  uint32_t offset = id % TABLE_BLOCK_SIZE;
  if(bitmap->is_set(offset))
  {
    --m_size;
    bitmap->unset(id % TABLE_BLOCK_SIZE);
  }
}

const Bitmap* 
BitTable::get_bitmap(uint32_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    bitmap = m_bitmaps.insert_new(bitset_id, TABLE_BLOCK_SIZE);
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
  BTree<Bitmap>::Iterator it = m_bitmaps.iterator();
  while(it.has_next())
  {
    it.next().p_value->all_zeros();
  }
  m_size = 0;
}

void
bittable_union(BitTable* first, const BitTable* second)
{
  BTree<Bitmap>::Iterator it = second->m_bitmaps.iterator();
  while(it.has_next())
  {
    BTree<Bitmap>::Entry entry = it.next();
    uint32_t start = entry.m_key * TABLE_BLOCK_SIZE;
    for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
    {
      uint32_t next_id = start+i;
      if(entry.p_value->is_set(i))
      {
        first->add(next_id);
      }
    }
  }
}

void
bittable_difference(BitTable* first, const BitTable* second)
{
  BTree<Bitmap>::Iterator it = second->m_bitmaps.iterator();
  while(it.has_next())
  {
    BTree<Bitmap>::Entry entry = it.next();
    uint32_t start = entry.m_key * TABLE_BLOCK_SIZE;
    for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
    {
      uint32_t next_id = start+i;
      if(entry.p_value->is_set(i))
      {
        first->remove(next_id);
      }
    }
  }
}

} /* furious
*/ 
