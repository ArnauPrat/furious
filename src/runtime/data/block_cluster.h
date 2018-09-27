
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "table.h"

#include <bitset>

#define FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{

class BlockCluster final
{
public:
  BlockCluster(TBlock* block);

  ~BlockCluster() = default;

  void 
  append(TBlock* block, 
         const std::string& type);

  void 
  append(const BlockCluster* other);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  
  TBlock*                         m_blocks[FURIOUS_MAX_CLUSTER_SIZE]; 
  size_t                          m_num_elements = 0;
  std::bitset<TABLE_BLOCK_SIZE>   m_enabled;
  int32_t                         m_start = -1;
};



  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
