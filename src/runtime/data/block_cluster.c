
#include "block_cluster.h"
#include "../../common/platform.h"
#include "txtable.h"
#include "tmptable.h"
#include "../../common/bitmap.h"
#include "txbitmap_utils.h"
#include "string.h"


#define FDB_INVALID_BLOCK_START 0xffffffff

void
fdb_bcluster_factory_init(struct fdb_bcluster_factory_t* bcluster_factory, 
                          struct fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  *bcluster_factory = (struct fdb_bcluster_factory_t){};
  fdb_bitmap_factory_init(&bcluster_factory->m_cbitmap_factory, 
                          FDB_TXTABLE_BLOCK_SIZE,
                          allocator);

  fdb_bitmap_factory_init(&bcluster_factory->m_glbitmap_factory, 
                          FDB_MAX_CLUSTER_SIZE,
                          allocator);
}

void
fdb_bcluster_factory_release(struct fdb_bcluster_factory_t* bcluster_factory)
{
  fdb_bitmap_factory_release(&bcluster_factory->m_cbitmap_factory);
  fdb_bitmap_factory_release(&bcluster_factory->m_glbitmap_factory);
}

void
fdb_bcluster_init(struct fdb_bcluster_t* bc, 
                  struct fdb_bcluster_factory_t* factory) 
{
  *bc = (struct fdb_bcluster_t){};
  bc->p_factory= factory;
  bc->m_num_columns = 0;
  bc->m_start = FDB_INVALID_BLOCK_START;
  fdb_bitmap_init(&bc->m_enabled,
                  &bc->p_factory->m_cbitmap_factory);
  fdb_bitmap_init(&bc->m_global,
                  &bc->p_factory->m_glbitmap_factory);
  memset(&bc->m_sizes,0, sizeof(uint32_t)*FDB_MAX_CLUSTER_SIZE);
  memset(&bc->p_blocks,0, sizeof(void*)*FDB_MAX_CLUSTER_SIZE);
}

void
fdb_bcluster_release(struct fdb_bcluster_t* bc)
{
  fdb_bitmap_release(&bc->m_enabled);
  fdb_bitmap_release(&bc->m_global);
}

void 
fdb_bcluster_append_txtable_block(struct fdb_bcluster_t* bc,
                                  struct fdb_txtable_block_t* block, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  bool write)
{
  FDB_ASSERT(bc->m_num_columns < FDB_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  FDB_ASSERT((bc->m_start == FDB_INVALID_BLOCK_START || bc->m_start == block->m_start ) && "Unaligned block cluster");

  if(bc->m_start == FDB_INVALID_BLOCK_START)
  {
    bc->m_start = block->m_start;
    fdb_bitmap_set_txbitmap(&bc->m_enabled, 
                            &block->m_enabled, 
                            tx, 
                            txtctx);
  }

  void* data = fdb_txpool_alloc_ptr(block->p_table->p_data_allocator, 
                                    tx, 
                                    txtctx, 
                                    block->m_data, 
                                    write);
  bc->m_sizes[bc->m_num_columns] = block->m_esize;
  bc->p_blocks[bc->m_num_columns] = data;
  bc->m_num_columns++;
  fdb_bitmap_set_txbitmap_and(&bc->m_enabled, 
                              &block->m_enabled, 
                              tx, 
                              txtctx);
}

void 
fdb_bcluster_append_tmptable_block(struct fdb_bcluster_t* bc,
                                   struct fdb_tmptable_block_t* block)
{
  FDB_ASSERT(bc->m_num_columns < FDB_MAX_CLUSTER_SIZE && "Cannot append block to full cluster");
  FDB_ASSERT((bc->m_start == FDB_INVALID_BLOCK_START || bc->m_start == block->m_start ) && "Unaligned block cluster");

  if(bc->m_start == FDB_INVALID_BLOCK_START)
  {
    bc->m_start = block->m_start;
    fdb_bitmap_set_bitmap(&bc->m_enabled, 
                            &block->m_enabled);
  }

  bc->m_sizes[bc->m_num_columns] = block->m_esize;
  bc->p_blocks[bc->m_num_columns] = block->p_data;
  bc->m_num_columns++;
  fdb_bitmap_set_and(&bc->m_enabled, 
                     &block->m_enabled);
}

void 
fdb_bcluster_append_global(struct fdb_bcluster_t* bc, 
                           void* global, 
                           size_t esize)
{
  fdb_bitmap_set(&bc->m_global, bc->m_num_columns);
  bc->p_blocks[bc->m_num_columns] = global;
  bc->m_sizes[bc->m_num_columns] = esize;
  bc->m_num_columns++;
}

void 
fdb_bcluster_append_cluster(FDB_RESTRICT(struct fdb_bcluster_t*) bc, 
                            FDB_RESTRICT(const struct fdb_bcluster_t*) other)
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
    bc->m_sizes[bc->m_num_columns] = other->m_sizes[i];
    if(fdb_bitmap_is_set(&other->m_global, i))
    {
      fdb_bitmap_set(&bc->m_global, bc->m_num_columns);
    }
    bc->m_num_columns++;
  }
}

void 
fdb_bcluster_filter(FDB_RESTRICT(struct fdb_bcluster_t*) bc, 
                    FDB_RESTRICT(const struct fdb_bcluster_t*) other)
{
  fdb_bitmap_set_and(&bc->m_enabled, &other->m_enabled);
}

bool
fdb_bcluster_has_elements(struct fdb_bcluster_t* bc)
{
  return (bc->m_enabled.m_num_set > 0) ||
  fdb_bitmap_is_set(&bc->m_global, 0);
}
