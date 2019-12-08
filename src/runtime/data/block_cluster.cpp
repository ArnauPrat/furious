
#include "block_cluster.h"
#include "../../common/platform.h"
#include "table.h"
#include "common/bitmap.h"

namespace furious
{

#define _FURIOUS_INVALID_BLOCK_START 0xffffffff

BlockCluster::BlockCluster() :
m_num_columns(0),
m_start(_FURIOUS_INVALID_BLOCK_START) 
{
  bitmap_init(&m_enabled);
  bitmap_init(&m_global);
}

BlockCluster::BlockCluster(TBlock* block) :
m_num_columns(1),
m_start(block->m_start) 
{
  bitmap_init(&m_enabled);
  bitmap_init(&m_global);

  bitmap_set_bitmap(&m_enabled, &block->m_enabled);
  m_blocks[0] = block;
}

BlockCluster::BlockCluster(const BlockCluster& block) : 
m_num_columns(block.m_num_columns),
m_start(block.m_start)
{
  bitmap_init(&m_enabled);
  bitmap_set_bitmap(&m_enabled, &block.m_enabled);
  bitmap_init(&m_global);
  bitmap_set_bitmap(&m_global, &block.m_global);

  for(uint32_t i = 0; i < block.m_num_columns; ++i)
  {
    m_blocks[i] = block.m_blocks[i];
  }
}

BlockCluster::~BlockCluster()
{
  bitmap_release(&m_enabled);
  bitmap_release(&m_global);
}

void BlockCluster::append(TBlock* block)
{
  FURIOUS_ASSERT(m_num_columns < _FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  FURIOUS_ASSERT((m_start == _FURIOUS_INVALID_BLOCK_START || m_start == block->m_start ) && "Unaligned block cluster");

  if(m_start == _FURIOUS_INVALID_BLOCK_START)
  {
    m_start = block->m_start;
    bitmap_set_bitmap(&m_enabled, &block->m_enabled);
  }

  m_blocks[m_num_columns] = block;
  m_num_columns++;
  bitmap_set_and(&m_enabled, &block->m_enabled);
}

void 
BlockCluster::append_global(void* global)
{
  bitmap_set(&m_global, m_num_columns);
  m_blocks[m_num_columns] = global;
  m_num_columns++;
}

void BlockCluster::append(const BlockCluster* other)
{

  FURIOUS_ASSERT(((m_start == other->m_start) || 
                  (m_start == _FURIOUS_INVALID_BLOCK_START || other->m_start == _FURIOUS_INVALID_BLOCK_START) )&&
                  "Trying to append unaligned blockclusters with different start values");

  if(other->m_start != _FURIOUS_INVALID_BLOCK_START)
  {
    if(m_start == _FURIOUS_INVALID_BLOCK_START)
    {
      bitmap_set_bitmap(&m_enabled, &other->m_enabled);
    }
    else
    {
      bitmap_set_and(&m_enabled, &other->m_enabled);
    }
    m_start = other->m_start;
  }

  for(size_t i = 0; i < other->m_num_columns; ++i)
  {
    FURIOUS_ASSERT(m_num_columns < _FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    m_blocks[m_num_columns] = other->m_blocks[i];
    if(bitmap_is_set(&other->m_global, i))
    {
      bitmap_set(&m_global, m_num_columns);
    }
    m_num_columns++;
  }
}

void BlockCluster::filter(const BlockCluster* other)
{
  bitmap_set_and(&m_enabled, &other->m_enabled);
}

TBlock* 
BlockCluster::get_tblock(uint32_t index) const
{
  FURIOUS_ASSERT(!bitmap_is_set(&m_global, index) && "Trying to get tblock which is a global from a BlockCluster");
  return (TBlock*)m_blocks[index];
}

void* 
BlockCluster::get_global(uint32_t index) const
{
  FURIOUS_ASSERT(bitmap_is_set(&m_global, index) && "Trying to get global which is a tblock from a BlockCluster");
  return m_blocks[index];
}

bool
BlockCluster::has_elements() const
{
  return (m_enabled.m_num_set > 0) ||
          bitmap_is_set(&m_global, 0);
}
  
} /* furious */ 
