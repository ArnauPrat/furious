

#include "block_cluster.h"

#include <cassert>

namespace furious
{

#define _FURIOUS_INVALID_BLOCK_START 0xffffffff

BlockCluster::BlockCluster(TBlock* block) :
m_num_elements(1),
p_enabled(nullptr),
m_start(block->m_start) 
{
  p_enabled = new Bitmap(block->p_enabled->max_bits());
  p_enabled->set_bitmap(block->p_enabled);
  m_blocks[0] = block;
}

BlockCluster::~BlockCluster()
{
  delete p_enabled;
}

void BlockCluster::append(TBlock* block, 
                          const std::string& type)
{
  assert(m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  assert((m_start == _FURIOUS_INVALID_BLOCK_START || m_start == block->m_start ) && "Unaligned block cluster");

  if(m_start == _FURIOUS_INVALID_BLOCK_START)
  {
    m_start = block->m_start;
  }

  m_blocks[m_num_elements] = block;
  m_num_elements++;
  p_enabled->set_and(block->p_enabled);
}

void BlockCluster::append(const BlockCluster* other)
{
  for(size_t i = 0; i < other->m_num_elements; ++i)
  {
    assert(m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    m_blocks[m_num_elements] = other->m_blocks[i];
    m_num_elements++;
  }
  p_enabled->set_and(other->p_enabled);
}
  
} /* furious */ 
