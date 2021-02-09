
#include "data_utils.h"
#include "tmpbittable.h"
#include "txbittable.h"
#include "txtable.h"
#include "tmptable.h"
#include "block_cluster.h"
#include "../../common/platform.h"

void
fdb_txtable_add_reference(struct fdb_txtable_t* table, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx, 
                          entity_id_t entity_a,
                          entity_id_t entity_b)
{
  FDB_ASSERT(table->m_esize == sizeof(entity_id_t) && "Attempting to add a reference to a non-reference txtable");
  entity_id_t* ptr = fdb_txtable_create_component(table, 
                                                tx, 
                                                txtctx, 
                                                entity_a);
  *ptr = entity_b;
}

bool
fdb_txtable_exists_reference(struct fdb_txtable_t* table, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx, 
                          entity_id_t entity_a,
                          entity_id_t entity_b)
{
  FDB_ASSERT(table->m_esize == sizeof(entity_id_t) && "Attempting to add a reference to a non-reference txtable");
  entity_id_t* ptr = fdb_txtable_get_component(table, 
                                                tx, 
                                                txtctx, 
                                                entity_a, 
                                                false);
  return ptr != NULL && *ptr == entity_b;
}


void
copy_component_ptr(struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx,uint32_t chunk_size, 
                   uint32_t stride,
                   entity_id_t source,
                   entity_id_t target,
                   FDB_RESTRICT(struct fdb_btree_t*)* hash_tables, 
                   FDB_RESTRICT(struct fdb_tmptable_t*)* tables,
                   uint32_t num_tables)
{
  uint32_t other_block_id = (target / FDB_TXTABLE_BLOCK_SIZE);
  uint32_t other_hash_table_index = fdb_block_get_offset(other_block_id, chunk_size, stride );
  struct fdb_btree_t* hash_table = hash_tables[other_hash_table_index];
  struct fdb_bcluster_t* cluster = (struct fdb_bcluster_t*)fdb_btree_get(hash_table, other_block_id);
  if(cluster != NULL)
  {
    if(fdb_bitmap_is_set(&cluster->m_enabled, target % FDB_TXTABLE_BLOCK_SIZE))
    {
      for(uint32_t i = 0; i < num_tables; ++i)
      {
        struct fdb_tmptable_t* table_ptr = tables[i];
        void* ptr = fdb_tmptable_create_component(table_ptr, source);
        size_t esize = cluster->m_sizes[i];
        void* data = cluster->p_blocks[i];
        void* object = &(((char*)data)[(target%FDB_TXTABLE_BLOCK_SIZE)*esize]);
        FDB_ASSERT(object != NULL && "Pointer to component cannot be null");
        *(size_t*)ptr = (size_t)object;
      }
    }
  }
}

void
find_roots_and_blacklist(struct fdb_bcluster_t*  block_cluster, 
                         FDB_RESTRICT(struct fdb_tmpbittable_t*) roots,
                         FDB_RESTRICT(struct fdb_tmpbittable_t*) blacklist)
{
  FDB_ASSERT(block_cluster->m_num_columns == 1 && "Find roots only works with single column block clusters");
  FDB_ASSERT(!fdb_bitmap_is_set(&block_cluster->m_global, 0) && "fdb_bcluster_t cannot be global");

  entity_id_t* data = block_cluster->p_blocks[0];
  for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)
  {
    uint32_t next_entity = block_cluster->m_start + i;
    if(fdb_bitmap_is_set(&block_cluster->m_enabled, i))
    {
      uint32_t other = data[i];
      fdb_tmpbittable_add(blacklist, next_entity);
      fdb_tmpbittable_remove(roots, next_entity);
      if(!fdb_tmpbittable_exists(blacklist, other))
      {
        fdb_tmpbittable_add(roots, other);
      }
    }
  }
}

void
filter_blacklists(FDB_RESTRICT(struct fdb_tmpbittable_t*) partial_lists[],
                  uint32_t cpartial_lists,
                  FDB_RESTRICT(struct fdb_tmpbittable_t*) current_frontier)
{
  for(uint32_t i = 0; i < cpartial_lists; ++i)
  {
    FDB_RESTRICT(struct fdb_tmpbittable_t*) partial_list = partial_lists[i];
    fdb_tmpbittable_difference(current_frontier, partial_list);
  }
}

void
frontiers_union(FDB_RESTRICT(struct fdb_tmpbittable_t*) next_frontiers[],
                uint32_t num_frontiers,
                FDB_RESTRICT(struct fdb_tmpbittable_t*) current_frontier)
{
  for(uint32_t i = 0; i < num_frontiers; ++i)
  {
    FDB_RESTRICT(struct fdb_tmpbittable_t*) partial_frontier = next_frontiers[i];
    fdb_tmpbittable_union(current_frontier, partial_frontier);
  }
}

void
filter_txbittable_exists(struct fdb_txbittable_t* bittable, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx, 
                         struct fdb_bcluster_t* block_cluster,
                         uint32_t column)
{
  for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster->p_blocks[column])[i];
      if(!fdb_txbittable_exists(bittable, tx, txtctx, id))
      {
        fdb_bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

void
filter_txbittable_not_exists(struct fdb_txbittable_t* bittable, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             struct fdb_bcluster_t* block_cluster,
                              uint32_t column)
{
  for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster->p_blocks[column])[i];
      if(fdb_txbittable_exists(bittable, tx, txtctx, id))
      {
        fdb_bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

void
gather(struct fdb_tx_t* tx, 
       struct fdb_txthread_ctx_t* txtctx,
       struct fdb_bcluster_t* cluster,
       FDB_RESTRICT(struct fdb_btree_t*) hash_tables[],
       uint32_t   chunk_size, 
       uint32_t   stride,
       FDB_RESTRICT(struct fdb_tmptable_t*)*  tables, 
       uint32_t   num_tables)
{
  FDB_ASSERT(cluster->m_num_columns == 1 && "Cluster passed to gather must have a single column of references");

  const struct fdb_bitmap_t* enabled = &cluster->m_enabled;
  const entity_id_t* ref_data = (entity_id_t*)cluster->p_blocks[0];
  for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(enabled, i))
    {
      entity_id_t id = cluster->m_start + i;
      entity_id_t target = ref_data[i];
      copy_component_ptr(tx, 
                         txtctx, 
                         chunk_size, 
                         stride, 
                         id, 
                         target, 
                         hash_tables, 
                         tables,
                         num_tables);
    }
  }
}

void
build_bcluster_from_refs(FDB_RESTRICT(struct fdb_bcluster_t*) ref_cluster,  
                         FDB_RESTRICT(struct fdb_tmpbittable_t*) current_frontier,
                         FDB_RESTRICT(struct fdb_tmpbittable_t*) next_frontier,
                         FDB_RESTRICT(struct fdb_bcluster_t*) cluster, 
                         FDB_RESTRICT(struct fdb_tmptable_t*)*  tables, 
                         uint32_t   num_tables)
{
  FDB_ASSERT(ref_cluster->m_num_columns == 1 && "The ref_cluster should contain a single column");

  uint32_t block_id = ref_cluster->m_start / FDB_TXTABLE_BLOCK_SIZE;
  for(uint32_t i = 0; i < num_tables; ++i)
  {
    struct fdb_tmptable_block_t* tmptable_block = fdb_tmptable_get_block(tables[i], 
                                                                         block_id);
    if(tmptable_block != NULL)
    {
      fdb_bcluster_append_tmptable_block(cluster, tmptable_block);
    }
  }
  FDB_ASSERT((cluster->m_num_columns == 0 || cluster->m_num_columns == num_tables) && "The block should eexist in all or in no tables");

  if(cluster->m_num_columns > 0)
  {
    entity_id_t* data = (entity_id_t*)ref_cluster->p_blocks[0];
    fdb_bitmap_nullify(&cluster->m_enabled);
    for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)
    {
      uint32_t source = i + ref_cluster->m_start;
      if(fdb_bitmap_is_set(&ref_cluster->m_enabled,i))
      {
        entity_id_t target = data[i];
        if(fdb_tmpbittable_exists(current_frontier, target))
        {
          fdb_bitmap_set(&cluster->m_enabled,i);
          FDB_ASSERT(!fdb_tmpbittable_exists(next_frontier, source) && "Source should not exist");
          fdb_tmpbittable_add(next_frontier, source);
        }
      }
    }
  }
  return;
}

