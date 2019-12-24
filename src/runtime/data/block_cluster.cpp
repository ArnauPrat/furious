
#include "block_cluster.h"
#include "../../common/platform.h"
#include "table.h"
#include "../../common/bitmap.h"

namespace furious
{

#define FURIOUS_INVALID_BLOCK_START 0xffffffff

block_cluster_t
block_cluster_create(mem_allocator_t* allocator) 
{

  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")
  block_cluster_t bc;
  bc.m_num_columns = 0;
  bc.m_start = FURIOUS_INVALID_BLOCK_START;
  bc.m_enabled = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, allocator != nullptr ? allocator : &global_mem_allocator);
  bc.m_global = bitmap_create(FURIOUS_MAX_CLUSTER_SIZE, allocator != nullptr ? allocator : &global_mem_allocator);
  memset(&bc.p_blocks,0, sizeof(void*)*FURIOUS_MAX_CLUSTER_SIZE);
  return bc;
}

void
block_cluster_destroy(block_cluster_t* bc, 
                      mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")

  bitmap_destroy(&bc->m_enabled, allocator != nullptr ? allocator : &global_mem_allocator);
  bitmap_destroy(&bc->m_global, allocator != nullptr ? allocator : &global_mem_allocator);
}

void 
block_cluster_append(block_cluster_t* bc,
                     TBlock* block)
{
  FURIOUS_ASSERT(bc->m_num_columns < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  FURIOUS_ASSERT((bc->m_start == FURIOUS_INVALID_BLOCK_START || bc->m_start == block->m_start ) && "Unaligned block cluster");

  if(bc->m_start == FURIOUS_INVALID_BLOCK_START)
  {
    bc->m_start = block->m_start;
    bitmap_set_bitmap(&bc->m_enabled, &block->m_enabled);
  }

  bc->p_blocks[bc->m_num_columns] = block;
  bc->m_num_columns++;
  bitmap_set_and(&bc->m_enabled, &block->m_enabled);
}

void 
block_cluster_append_global(block_cluster_t* bc, 
                            void* global)
{
  bitmap_set(&bc->m_global, bc->m_num_columns);
  bc->p_blocks[bc->m_num_columns] = global;
  bc->m_num_columns++;
}

void 
block_cluster_append(FURIOUS_RESTRICT(block_cluster_t*) bc, 
                     FURIOUS_RESTRICT(const block_cluster_t*) other)
{
  FURIOUS_ASSERT(((bc->m_start == other->m_start) || 
                  (bc->m_start == FURIOUS_INVALID_BLOCK_START || other->m_start == FURIOUS_INVALID_BLOCK_START) )&&
                  "Trying to append unaligned blockclusters with different start values");

  if(other->m_start != FURIOUS_INVALID_BLOCK_START)
  {
    if(bc->m_start == FURIOUS_INVALID_BLOCK_START)
    {
      bitmap_set_bitmap(&bc->m_enabled, &other->m_enabled);
    }
    else
    {
      bitmap_set_and(&bc->m_enabled, &other->m_enabled);
    }
    bc->m_start = other->m_start;
  }

  for(size_t i = 0; i < other->m_num_columns; ++i)
  {
    FURIOUS_ASSERT(bc->m_num_columns < FURIOUS_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    bc->p_blocks[bc->m_num_columns] = other->p_blocks[i];
    if(bitmap_is_set(&other->m_global, i))
    {
      bitmap_set(&bc->m_global, bc->m_num_columns);
    }
    bc->m_num_columns++;
  }
}

void 
block_cluster_filter(FURIOUS_RESTRICT(block_cluster_t*) bc, 
                     FURIOUS_RESTRICT(const block_cluster_t*) other)
{
  bitmap_set_and(&bc->m_enabled, &other->m_enabled);
}

TBlock* 
block_cluster_get_tblock(block_cluster_t* bc, 
                         uint32_t index)
{
  FURIOUS_ASSERT(!bitmap_is_set(&bc->m_global, index) && "Trying to get tblock which is a global from a BlockCluster");
  return (TBlock*)bc->p_blocks[index];
}

void* 
block_cluster_get_global(block_cluster_t* bc, 
                         uint32_t index)
{
  FURIOUS_ASSERT(bitmap_is_set(&bc->m_global, index) && "Trying to get global which is a tblock from a BlockCluster");
  return bc->p_blocks[index];
}

bool
block_cluster_has_elements(block_cluster_t* bc)
{
  return (bc->m_enabled.m_num_set > 0) ||
          bitmap_is_set(&bc->m_global, 0);
}
  
} /* furious */ 
