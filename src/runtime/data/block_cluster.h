
#ifndef _FURIOUS_BLOCK_CLUSTER_H_H
#define _FURIOUS_BLOCK_CLUSTER_H_H value

#include "../../common/bitmap.h"
#include "../../common/types.h"
#include "common.h"

#define FURIOUS_MAX_CLUSTER_SIZE 16

namespace furious
{

struct table_block_t;

/**
 * \brief This is used to represent a joined block of data, mostly table blocks.
 * However, it can also contain pointers to global data.
 */
struct block_cluster_t 
{
  uint32_t         m_num_columns;
  bitmap_t         m_enabled;
  bitmap_t         m_global;
  uint32_t         m_start;
  void*            p_blocks[FURIOUS_MAX_CLUSTER_SIZE]; 
};

/**
 * \brief Creates a block cluster
 *
 * \param allocator The allocator to use
 *
 * \return Returns a block cluster
 */
block_cluster_t
block_cluster_create(mem_allocator_t* allocator);

/**
 * \brief Destroys a block cluster
 *
 * \param bc The block cluster to destroy
 */
void
block_cluster_destroy(block_cluster_t* bc, 
                      mem_allocator_t* allocator);


void 
block_cluster_append(block_cluster_t* bc, 
                     table_block_t* block);

void 
block_cluster_append_global(block_cluster_t* bc, 
                            void* global);

void 
block_cluster_append(FURIOUS_RESTRICT(block_cluster_t*) bc,
                     FURIOUS_RESTRICT(const block_cluster_t*) other);

void 
block_cluster_filter(FURIOUS_RESTRICT(block_cluster_t*) bc,
                     FURIOUS_RESTRICT(const block_cluster_t*) other);

table_block_t* 
block_cluster_get_tblock(block_cluster_t* bc,
                         uint32_t index);

void* 
block_cluster_get_global(block_cluster_t* bc,
                         uint32_t index);

bool
block_cluster_has_elements(block_cluster_t* bc);


} /* furious
*/ 

#endif /* ifndef _FURIOUS_BLOCK_CLUSTER_H_H */
