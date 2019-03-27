
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "table.h"

#define FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{

class BlockCluster final
{
public:
  BlockCluster();
  BlockCluster(TBlock* block);
  BlockCluster(const BlockCluster& block);

  ~BlockCluster();

  void 
  append(TBlock* block);

  void 
  append(const BlockCluster* other);

  void 
  filter(const BlockCluster* other);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  
  TBlock*                         m_blocks[FURIOUS_MAX_CLUSTER_SIZE]; 
  size_t                          m_num_elements;
  Bitmap*                         p_enabled;
  uint32_t                        m_start;
};




  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
