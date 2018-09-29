
#include "bit_table.h"
#include <cassert>

namespace furious
{

bool
BitTable::exists(int32_t id) const
{
  int32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    return false;
  }

  const std::bitset<TABLE_BLOCK_SIZE>& set = it->second;

  return set[id - bitset_id];
}

void
BitTable::add(int32_t id)
{
  int32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    m_bitsets[bitset_id] = std::bitset<TABLE_BLOCK_SIZE>();
  }

  std::bitset<TABLE_BLOCK_SIZE>& set = m_bitsets[bitset_id];
  set[id - bitset_id] = true;
}

void
BitTable::remove(int32_t id)
{
  int32_t bitset_id = id / TABLE_BLOCK_SIZE;
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    return;
  }

  std::bitset<TABLE_BLOCK_SIZE>& set = m_bitsets[bitset_id];
  set[id - bitset_id] = false;
}

const std::bitset<TABLE_BLOCK_SIZE>&
BitTable::get_bitset(int32_t bitset_id) const
{
  assert(bitset_id % TABLE_BLOCK_SIZE == 0 && "Invalid bitset_id on bit table");
  auto it = m_bitsets.find(bitset_id);
  if(it == m_bitsets.end()) 
  {
    m_bitsets[bitset_id] = std::bitset<TABLE_BLOCK_SIZE>();
  }
  return m_bitsets[bitset_id];
}

} /* furious
*/ 
