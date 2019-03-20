
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
  return bitmap->is_set(id - bitset_id);
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
    m_bitmaps.insert_new(bitset_id, TABLE_BLOCK_SIZE);
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

} /* furious
*/ 
