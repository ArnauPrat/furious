

#include "block_cluster.h"

#include <cassert>

namespace furious
{

void BlockCluster::append(TBlock* block, 
                          const std::string& type)
{
  assert(m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  assert((m_start == -1 || m_start == block->m_start ) && "Unaligned block cluster");

  if(m_start == -1)
  {
    m_start = block->m_start;
  }

  m_blocks[m_num_elements] = block;
  m_num_elements++;
  m_enabled &= block->m_enabled;
}

void BlockCluster::append(const BlockCluster* other)
{
  for(size_t i = 0; i < other->m_num_elements; ++i)
  {
    assert(m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    m_blocks[m_num_elements] = other->m_blocks[i];
    m_num_elements++;
  }
  m_enabled &= other->m_enabled;
}
  
} /* furious */ 
