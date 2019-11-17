
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "table.h"

#define _FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{


/**
 * \brief This is used to represent a joined block of data, mostly table blocks.
 * However, it can also contain pointers to global data.
 */
struct BlockCluster 
{
  BlockCluster();
  BlockCluster(TBlock* block);
  BlockCluster(const BlockCluster& block);

  ~BlockCluster();

  void 
  append(TBlock* block);

  void 
  append_global(void* global);

  void 
  append(const BlockCluster* other);

  void 
  filter(const BlockCluster* other);

  TBlock* 
  get_tblock(uint32_t index) const;

  void* 
  get_global(uint32_t index) const;

  bool
  has_elements() const;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  
  size_t                          m_num_columns;
  Bitmap*                         p_enabled;
  Bitmap*                         p_global;
  uint32_t                        m_start;

private:
  void*                           m_blocks[_FURIOUS_MAX_CLUSTER_SIZE]; 
};




  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
