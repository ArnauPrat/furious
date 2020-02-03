
#include "block_cluster.h"
#include "../../common/platform.h"
#include "table.h"
#include "../../common/bitmap.h"
#include "string.h"


#define FDB_INVALID_BLOCK_START 0xffffffff

void
fdb_bcluster_init(fdb_bcluster_t* bc, fdb_mem_allocator_t* allocator) 
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  bc->m_num_columns = 0;
  bc->m_start = FDB_INVALID_BLOCK_START;
  fdb_bitmap_init(&bc->m_enabled, 
                  FDB_TABLE_BLOCK_SIZE, 
                  allocator != NULL ? allocator : fdb_get_global_mem_allocator());
  fdb_bitmap_init(&bc->m_global, 
                  FDB_MAX_CLUSTER_SIZE, 
                  allocator != NULL ? allocator : fdb_get_global_mem_allocator());
  memset(&bc->p_blocks,0, sizeof(void*)*FDB_MAX_CLUSTER_SIZE);
}

void
fdb_bcluster_release(fdb_bcluster_t* bc, 
                     fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  fdb_bitmap_release(&bc->m_enabled, allocator != NULL ? allocator : fdb_get_global_mem_allocator());
  fdb_bitmap_release(&bc->m_global, allocator != NULL ? allocator : fdb_get_global_mem_allocator());
}

void 
fdb_bcluster_append_block(fdb_bcluster_t* bc,
                          fdb_table_block_t* block)
{
  FDB_ASSERT(bc->m_num_columns < FDB_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  FDB_ASSERT((bc->m_start == FDB_INVALID_BLOCK_START || bc->m_start == block->m_start ) && "Unaligned block cluster");

  if(bc->m_start == FDB_INVALID_BLOCK_START)
  {
    bc->m_start = block->m_start;
    fdb_bitmap_set_bitmap(&bc->m_enabled, &block->m_enabled);
  }

  bc->p_blocks[bc->m_num_columns] = block;
  bc->m_num_columns++;
  fdb_bitmap_set_and(&bc->m_enabled, &block->m_enabled);
}

void 
fdb_bcluster_append_global(fdb_bcluster_t* bc, 
                           void* global)
{
  fdb_bitmap_set(&bc->m_global, bc->m_num_columns);
  bc->p_blocks[bc->m_num_columns] = global;
  bc->m_num_columns++;
}

void 
fdb_bcluster_append_cluster(FDB_RESTRICT(fdb_bcluster_t*) bc, 
                            FDB_RESTRICT(const fdb_bcluster_t*) other)
{
  FDB_ASSERT(((bc->m_start == other->m_start) || 
              (bc->m_start == FDB_INVALID_BLOCK_START || other->m_start == FDB_INVALID_BLOCK_START) )&&
             "Trying to append unaligned blockclusters with different start values");

  if(other->m_start != FDB_INVALID_BLOCK_START)
  {
    if(bc->m_start == FDB_INVALID_BLOCK_START)
    {
      fdb_bitmap_set_bitmap(&bc->m_enabled, &other->m_enabled);
    }
    else
    {
      fdb_bitmap_set_and(&bc->m_enabled, &other->m_enabled);
    }
    bc->m_start = other->m_start;
  }

  for(size_t i = 0; i < other->m_num_columns; ++i)
  {
    FDB_ASSERT(bc->m_num_columns < FDB_MAX_CLUSTER_SIZE && "Cannot append cluster. Not enough room");
    bc->p_blocks[bc->m_num_columns] = other->p_blocks[i];
    if(fdb_bitmap_is_set(&other->m_global, i))
    {
      fdb_bitmap_set(&bc->m_global, bc->m_num_columns);
    }
    bc->m_num_columns++;
  }
}

void 
fdb_bcluster_filter(FDB_RESTRICT(fdb_bcluster_t*) bc, 
                    FDB_RESTRICT(const fdb_bcluster_t*) other)
{
  fdb_bitmap_set_and(&bc->m_enabled, &other->m_enabled);
}

fdb_table_block_t* 
fdb_bcluster_get_tblock(fdb_bcluster_t* bc, 
                        uint32_t index)
{
  FDB_ASSERT(!fdb_bitmap_is_set(&bc->m_global, index) && "Trying to get fdb_table_block_t which is a global from a BlockCluster");
  return (fdb_table_block_t*)bc->p_blocks[index];
}

void* 
fdb_bcluster_get_global(fdb_bcluster_t* bc, 
                        uint32_t index)
{
  FDB_ASSERT(fdb_bitmap_is_set(&bc->m_global, index) && "Trying to get global which is a fdb_table_block_t from a BlockCluster");
  return bc->p_blocks[index];
}

bool
fdb_bcluster_has_elements(fdb_bcluster_t* bc)
{
  return (bc->m_enabled.m_num_set > 0) ||
  fdb_bitmap_is_set(&bc->m_global, 0);
}
