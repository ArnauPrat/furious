

#include "block_cluster.h"

#include <cassert>

namespace furious
{

void append(BlockCluster* self, 
            TBlock* block)
{
  assert(self->m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  assert((self->m_start == -1 || self->m_start == block->m_start ) && "Unaligned block cluster");

  if(self->m_start == -1)
  {
    self->m_start = block->m_start;
  }

  self->m_blocks[self->m_num_elements] = block;
  self->m_num_elements++;
  self->m_enabled &= block->m_enabled;
}

void append(BlockCluster* self, 
            const BlockCluster* other)
{
  for(size_t i = 0; i < other->m_num_elements; ++i)
  {
    assert(self->m_num_elements < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    self->m_blocks[self->m_num_elements] = other->m_blocks[i];
    self->m_num_elements++;
  }
  self->m_enabled &= other->m_enabled;
}
  
} /* furious
 */ 
