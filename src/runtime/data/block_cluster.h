
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "../../common/bitmap.h"
#include "../../common/types.h"
#include "common.h"

#define _FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{

struct TBlock;

using bc_enabled_t = bitmap_t<TABLE_BLOCK_SIZE>;
using bc_global_t = bitmap_t<_FURIOUS_MAX_CLUSTER_SIZE>;


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
  
  uint32_t         m_num_columns;
  bc_enabled_t     m_enabled;
  bc_global_t      m_global;
  uint32_t         m_start;

private:
  void*            m_blocks[_FURIOUS_MAX_CLUSTER_SIZE]; 
};




  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
