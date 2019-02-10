
#include "bit_table.h"
#include "table.h"

#include <assert.h>

namespace furious
{

BitTable::~BitTable()
{
}

bool
BitTable::exists(uint32_t id) const
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
BitTable::add(uint32_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    bitmap = m_bitmaps.insert_new(bitset_id,TABLE_BLOCK_SIZE);
  }

  bitmap->set(id % TABLE_BLOCK_SIZE);
}

void
BitTable::remove(uint32_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    return;
  }
  bitmap->unset(id % TABLE_BLOCK_SIZE);
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

} /* furious
*/ 
