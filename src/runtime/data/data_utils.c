
#include "data_utils.h"
#include "bittable.h"
#include "block_cluster.h"
#include "../../common/platform.h"


void
copy_component_ptr(uint32_t chunk_size, 
                   uint32_t stride,
                   entity_id_t source,
                   entity_id_t target,
                   FDB_RESTRICT(fdb_btree_t*)* hash_tables, 
                   FDB_RESTRICT(fdb_table_t*)* tables,
                   uint32_t num_tables)
{
  uint32_t other_block_id = (target / FDB_TABLE_BLOCK_SIZE);
  uint32_t other_hash_table_index = block_get_offset(other_block_id, chunk_size, stride );
  fdb_btree_t* hash_table = hash_tables[other_hash_table_index];
  fdb_bcluster_t* cluster = (fdb_bcluster_t*)fdb_btree_get(hash_table, other_block_id);
  if(cluster != NULL)
  {
    if(fdb_bitmap_is_set(&cluster->m_enabled, target % FDB_TABLE_BLOCK_SIZE))
    {
      for(uint32_t i = 0; i < num_tables; ++i)
      {
        fdb_table_t* table_ptr = tables[i];
        void* ptr = fdb_table_create_component(table_ptr, source);
        size_t esize = fdb_bcluster_get_tblock(cluster,i)->m_esize;
        void* object = &(((char*)fdb_bcluster_get_tblock(cluster,i)->p_data)[(target%FDB_TABLE_BLOCK_SIZE)*esize]);
        FDB_ASSERT(object != NULL && "Pointer to component cannot be null");
        *(size_t*)ptr = (size_t)object;
      }
    }
  }
}

void
find_roots_and_blacklist(fdb_bcluster_t*  block_cluster, 
                         FDB_RESTRICT(fdb_bittable_t*) roots,
                         FDB_RESTRICT(fdb_bittable_t*) blacklist)
{
  FDB_ASSERT(block_cluster->m_num_columns == 1 && "Find roots only works with single column block clusters");
  FDB_ASSERT(!fdb_bitmap_is_set(&block_cluster->m_global, 0) && "fdb_bcluster_t cannot be global");

  fdb_table_block_t* table_block = fdb_bcluster_get_tblock(block_cluster,0);
  entity_id_t* data = (entity_id_t*)table_block->p_data;
  for(uint32_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i)
  {
    uint32_t next_entity = table_block->m_start + i;
    if(fdb_bitmap_is_set(&table_block->m_enabled, i))
    {
      uint32_t other = data[i];
      fdb_bittable_add(blacklist, next_entity);
      fdb_bittable_remove(roots, next_entity);
      if(!fdb_bittable_exists(blacklist, other))
      {
        fdb_bittable_add(roots, other);
      }
    }
  }
}

void
filter_blacklists(FDB_RESTRICT(fdb_bittable_t*) partial_lists[],
                  uint32_t cpartial_lists,
                  FDB_RESTRICT(fdb_bittable_t*) current_frontier)
{
  for(uint32_t i = 0; i < cpartial_lists; ++i)
  {
    FDB_RESTRICT(fdb_bittable_t*) partial_list = partial_lists[i];
    fdb_bittable_difference(current_frontier, partial_list);
  }
}

void
frontiers_union(FDB_RESTRICT(fdb_bittable_t*) next_frontiers[],
                uint32_t num_frontiers,
                FDB_RESTRICT(fdb_bittable_t*) current_frontier)
{
  for(uint32_t i = 0; i < num_frontiers; ++i)
  {
    FDB_RESTRICT(fdb_bittable_t*) partial_frontier = next_frontiers[i];
    fdb_bittable_union(current_frontier, partial_frontier);
  }
}

void
filter_bittable_exists(const fdb_bittable_t* bittable, 
                       fdb_bcluster_t* block_cluster,
                       uint32_t column)
{
  for(uint32_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)fdb_bcluster_get_tblock(block_cluster, column)->p_data)[i];
      if(!fdb_bittable_exists(bittable, id))
      {
        fdb_bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

void
filter_bittable_not_exists(const fdb_bittable_t* bittable, 
                           fdb_bcluster_t* block_cluster,
                           uint32_t column)
{
  for(uint32_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)fdb_bcluster_get_tblock(block_cluster, column)->p_data)[i];
      if(fdb_bittable_exists(bittable, id))
      {
        fdb_bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

void
gather(fdb_bcluster_t* cluster,
       FDB_RESTRICT(fdb_btree_t*) hash_tables[],
       uint32_t   chunk_size, 
       uint32_t   stride,
       FDB_RESTRICT(fdb_table_t*)*  tables, 
       uint32_t   num_tables)
{
  FDB_ASSERT(cluster->m_num_columns == 1 && "Cluster passed to gather must have a single column of references");

  const fdb_bitmap_t* enabled = &cluster->m_enabled;
  const entity_id_t* ref_data = (entity_id_t*)fdb_bcluster_get_tblock(cluster,0)->p_data;
  for(uint32_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i)
  {
    if(fdb_bitmap_is_set(enabled, i))
    {
      entity_id_t id = cluster->m_start + i;
      entity_id_t target = ref_data[i];
      copy_component_ptr(chunk_size, 
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
build_fdb_bcluster_from_refs(FDB_RESTRICT(fdb_bcluster_t*) ref_cluster,  
                              FDB_RESTRICT(const fdb_bittable_t*) current_frontier,
                              FDB_RESTRICT(fdb_bittable_t*) next_frontier,
                              FDB_RESTRICT(fdb_bcluster_t*) cluster, 
                              FDB_RESTRICT(fdb_table_t*)*  tables, 
                              uint32_t   num_tables)
{
  FDB_ASSERT(ref_cluster->m_num_columns == 1 && "The ref_cluster should contain a single column");

  uint32_t block_id = ref_cluster->m_start / FDB_TABLE_BLOCK_SIZE;
  for(uint32_t i = 0; i < num_tables; ++i)
  {
    FDB_RESTRICT(fdb_table_block_t*) table_block_t = fdb_table_get_block(tables[i], block_id);
    if(table_block_t != NULL)
    {
      fdb_bcluster_append_block(cluster, table_block_t);
    }
  }
  FDB_ASSERT((cluster->m_num_columns == 0 || cluster->m_num_columns == num_tables) && "The block should eexist in all or in no tables");

  if(cluster->m_num_columns > 0)
  {
    entity_id_t* data = (entity_id_t*)fdb_bcluster_get_tblock(ref_cluster,0)->p_data;
    fdb_bitmap_nullify(&cluster->m_enabled);
    for(uint32_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i)
    {
      uint32_t source = i + ref_cluster->m_start;
      if(fdb_bitmap_is_set(&ref_cluster->m_enabled,i))
      {
        entity_id_t target = data[i];
        if(fdb_bittable_exists(current_frontier, target))
        {
          fdb_bitmap_set(&cluster->m_enabled,i);
          FDB_ASSERT(!fdb_bittable_exists(next_frontier, source) && "Source should not exist");
          fdb_bittable_add(next_frontier, source);
        }
      }
    }
  }
  return;
}

