
#include "bit_table.h"
#include "table.h"

#include <assert.h>

namespace furious
{

BitTable::~BitTable()
{
  for(std::unordered_map<uint32_t, Bitmap*>::iterator it = m_bitsets.begin();
      it != m_bitsets.end(); 
      ++it)
  {
    delete it->second;
  }
}

bool
BitTable::exists(uint32_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    return false;
  }

  const Bitmap* bitmap = it->second;

  return bitmap->is_set(id - bitset_id);
}

void
BitTable::add(uint32_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    m_bitsets[bitset_id] = new Bitmap(TABLE_BLOCK_SIZE);
  }

  Bitmap* set = m_bitsets[bitset_id];
  set->set(id % TABLE_BLOCK_SIZE);
}

void
BitTable::remove(uint32_t id)
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    return;
  }

  Bitmap* set = m_bitsets[bitset_id];
  set->unset(id % TABLE_BLOCK_SIZE);
}

const Bitmap* 
BitTable::get_bitmap(uint32_t id) const
{
  uint32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    m_bitsets[bitset_id] = new Bitmap(TABLE_BLOCK_SIZE);
  }
  return m_bitsets[bitset_id];
}

} /* furious
*/ 
