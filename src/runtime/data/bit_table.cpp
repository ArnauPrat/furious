
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
  Bitmap* bitmap = get_bitset(bitset_id);
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
BitTable::get_bitmap(entity_id_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  Bitmap* bitmap = get_bitset(bitset_id);
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
BitTable::apply_bitset(uint32_t id,
                     const Bitmap* bitmap,
                     logic_operation_t operation)
{
  Bitmap* bm = get_bitset(id);
  switch(operation)
  {
    case logic_operation_t::E_AND:
      if(bm != nullptr)
      {
        m_size -= bm->num_set();
        bm->set_and(bitmap);
        m_size += bm->num_set();
      }
      break;
    case logic_operation_t::E_OR:
      if(bm != nullptr)
      {
        m_size -= bm->num_set();
        bm->set_or(bitmap);
        m_size += bm->num_set();
      }
      else
      {
        Bitmap* nbitmap = m_bitmaps.insert_new(id,TABLE_BLOCK_SIZE);
        nbitmap->set_or(bitmap);
        m_size += nbitmap->num_set();
      }
      break;
    case logic_operation_t::E_DIFF:
      if(bm != nullptr)
      {
        m_size -= bm->num_set();
        bm->set_diff(bitmap);
        m_size += bm->num_set();
      }
      break;
  };
}

Bitmap*
BitTable::get_bitset(uint32_t bitset_id) const
{
  return m_bitmaps.get(bitset_id);
}

void
bittable_union(BitTable* first, const BitTable* second)
{
  BTree<Bitmap>::Iterator it = second->m_bitmaps.iterator();
  while(it.has_next())
  {
    BTree<Bitmap>::Entry entry = it.next();
    first->apply_bitset(entry.m_key, 
                        entry.p_value, 
                        BitTable::logic_operation_t::E_OR);
  }
}

void
bittable_difference(BitTable* first, const BitTable* second)
{
  BTree<Bitmap>::Iterator it = second->m_bitmaps.iterator();
  while(it.has_next())
  {
    BTree<Bitmap>::Entry entry = it.next();
    first->apply_bitset(entry.m_key, 
                        entry.p_value, 
                        BitTable::logic_operation_t::E_DIFF);
  }
}

} /* furious
*/ 
