

#include "block_cluster.h"

#include <assert.h>

namespace furious
{

#define _FURIOUS_INVALID_BLOCK_START 0xffffffff

BlockCluster::BlockCluster() :
m_num_elements(0),
p_enabled(nullptr),
p_global(nullptr),
m_start(_FURIOUS_INVALID_BLOCK_START) 
{
}

BlockCluster::BlockCluster(TBlock* block) :
m_num_elements(1),
p_enabled(nullptr),
p_global(nullptr),
m_start(block->m_start) 
{
  p_enabled = new Bitmap(block->p_enabled->max_bits());
  p_global = new Bitmap(_FURIOUS_MAX_CLUSTER_SIZE);
  p_enabled->set_bitmap(block->p_enabled);
  m_blocks[0] = block;
}

BlockCluster::BlockCluster(const BlockCluster& block) : 
m_num_elements(block.m_num_elements),
p_enabled(nullptr),
p_global(nullptr),
m_start(block.m_start)
{
  if(block.p_enabled != nullptr)
  {
    p_enabled = new Bitmap(block.p_enabled->max_bits());
    p_enabled->set_bitmap(block.p_enabled);
  }
  if(block.p_global != nullptr)
  {
    p_global = new Bitmap(_FURIOUS_MAX_CLUSTER_SIZE);
    p_global->set_bitmap(block.p_global);
  }
  for(uint32_t i = 0; i < block.m_num_elements; ++i)
  {
    m_blocks[i] = block.m_blocks[i];
  }
}

BlockCluster::~BlockCluster()
{

  if(p_enabled != nullptr)
  {
    delete p_enabled;
  }

  if(p_global != nullptr)
  {
    delete p_global;
  }
}

void BlockCluster::append(TBlock* block)
{
  assert(m_num_elements < _FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  assert((m_start == _FURIOUS_INVALID_BLOCK_START || m_start == block->m_start ) && "Unaligned block cluster");

  if(m_start == _FURIOUS_INVALID_BLOCK_START)
  {
    m_start = block->m_start;
  }
  if(p_enabled == nullptr)
  {
    p_enabled = new Bitmap(block->p_enabled->max_bits());
    p_enabled->set_bitmap(block->p_enabled);
  }

  if(p_global == nullptr)
  {
    p_global = new Bitmap(_FURIOUS_MAX_CLUSTER_SIZE);
  }

  m_blocks[m_num_elements] = block;
  m_num_elements++;
  p_enabled->set_and(block->p_enabled);
}

void 
BlockCluster::append_global(void* global)
{
  if(p_global == nullptr)
  {
    p_global = new Bitmap(_FURIOUS_MAX_CLUSTER_SIZE);
  }
  p_global->set(m_num_elements);
  m_blocks[m_num_elements] = global;
  m_num_elements++;
}

void BlockCluster::append(const BlockCluster* other)
{
  if(m_start == _FURIOUS_INVALID_BLOCK_START)
  {
    // We need to check this because either the blockcluster is empty or it just
    // has globals
    m_start = other->m_start;
  }

  assert((m_start == other->m_start || 
          (other->m_start == _FURIOUS_INVALID_BLOCK_START && other->p_global->is_set(0) && other->m_num_elements == 1)) 
         && "Trying to append unaligned blockclusters with different start values");

  if(other->p_enabled != nullptr)
  {
    if(p_enabled == nullptr)
    {
      p_enabled = new Bitmap(other->p_enabled->max_bits());
      p_enabled->set_bitmap(other->p_enabled);
    }
    else
    {
      p_enabled->set_and(other->p_enabled);
    }
  }

  if(p_global == nullptr)
  {
    p_global = new Bitmap(_FURIOUS_MAX_CLUSTER_SIZE);
  }
  for(size_t i = 0; i < other->m_num_elements; ++i)
  {
    assert(m_num_elements < _FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    m_blocks[m_num_elements] = other->m_blocks[i];
    if(other->p_global->is_set(i))
    {
      p_global->set(m_num_elements);
    }
    m_num_elements++;
  }
}

void BlockCluster::filter(const BlockCluster* other)
{
  if(p_enabled == nullptr)
  {
    p_enabled = new Bitmap(other->p_enabled->max_bits());
  }
  p_enabled->set_and(other->p_enabled);
}

TBlock* 
BlockCluster::get_tblock(uint32_t index) const
{
  assert(!p_global->is_set(index) && "Trying to get tblock which is a global from a BlockCluster");
  return (TBlock*)m_blocks[index];
}

void* 
BlockCluster::get_global(uint32_t index) const
{
  assert(p_global->is_set(index) && "Trying to get global which is a tblock from a BlockCluster");
  return m_blocks[index];
}

bool
BlockCluster::has_elements() const
{
  return (p_enabled != nullptr && p_enabled->num_set() > 0) ||
  (p_enabled == nullptr && p_global->is_set(0));
}
  
} /* furious */ 
