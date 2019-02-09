
#include "bit_table.h"
#include "table.h"

#include <assert.h>

namespace furious
{

BitTable::~BitTable()
{
  BTree<Bitmap*>::Iterator it = m_bitmaps.iterator();
  while(it.has_next())
  {
    delete *it.next();
  }
}

bool
BitTable::exists(uint32_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = *m_bitmaps.get(bitset_id);
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
  Bitmap** bitmap = m_bitmaps.get(bitset_id);
  Bitmap* bitmap_ptr = nullptr;
  if(bitmap == nullptr) 
  {
    bitmap_ptr = new Bitmap(TABLE_BLOCK_SIZE);
    m_bitmaps.insert(bitset_id,&bitmap_ptr);
  }
  else
  {
    bitmap_ptr = *bitmap;
  }

  bitmap_ptr->set(id % TABLE_BLOCK_SIZE);
}

void
BitTable::remove(uint32_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap** bitmap = m_bitmaps.get(bitset_id);
  if(bitmap == nullptr) 
  {
    return;
  }
  (*bitmap)->unset(id % TABLE_BLOCK_SIZE);
}

const Bitmap* 
BitTable::get_bitmap(uint32_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap** bitmap = m_bitmaps.get(bitset_id);
  Bitmap* bitmap_ptr = nullptr;
  if(bitmap == nullptr) 
  {
    bitmap_ptr = new Bitmap(TABLE_BLOCK_SIZE);
    m_bitmaps.insert(bitset_id, &bitmap_ptr);
  }
  else
  {
    bitmap_ptr = *bitmap;
  }
  return bitmap_ptr;
}

} /* furious
*/ 
