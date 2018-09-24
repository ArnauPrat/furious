
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "table.h"

#include <array>
#include <bitset>


#define FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{

struct BlockCluster 
{
  std::array<TBlock*,FURIOUS_MAX_CLUSTER_SIZE> m_blocks; 
  size_t                                       m_num_elements = 0;
  std::bitset<TABLE_BLOCK_SIZE>                m_enabled;
  int32_t                                      m_start = -1;
};

void 
append(BlockCluster* self, 
       TBlock* block);

void 
append(BlockCluster* self, 
       const BlockCluster* other);


  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
