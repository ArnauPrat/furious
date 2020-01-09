

#include "data_utils.h"
#include "bit_table.h"
#include "../../common/platform.h"

namespace furious
{

void
copy_component_ptr(uint32_t chunk_size, 
                   uint32_t stride,
                   entity_id_t source,
                   entity_id_t target,
                   const DynArray<FURIOUS_RESTRICT(btree_t*)>* hash_tables, 
                   FURIOUS_RESTRICT(table_t*)* tables,
                   uint32_t num_tables)
{
  uint32_t other_block_id = (target / FURIOUS_TABLE_BLOCK_SIZE);
  uint32_t other_hash_table_index = block_get_offset(other_block_id, chunk_size, stride );
  btree_t* hash_table = (*hash_tables)[other_hash_table_index];
  block_cluster_t* cluster = (block_cluster_t*)btree_get(hash_table, other_block_id);
  if(cluster != nullptr)
  {
    if(bitmap_is_set(&cluster->m_enabled, target % FURIOUS_TABLE_BLOCK_SIZE))
    {
      for(uint32_t i = 0; i < num_tables; ++i)
      {
        table_t* table_ptr = tables[i];
        void* ptr = table_alloc_component(table_ptr, source);
        size_t esize = block_cluster_get_tblock(cluster,i)->m_esize;
        void* object = &(((char*)block_cluster_get_tblock(cluster,i)->p_data)[(target%FURIOUS_TABLE_BLOCK_SIZE)*esize]);
        FURIOUS_ASSERT(object != nullptr && "Pointer to component cannot be null");
        *(size_t*)ptr = (size_t)object;
      }
    }
  }
}

void
find_roots_and_blacklist(block_cluster_t*  block_cluster, 
                         FURIOUS_RESTRICT(BitTable*) roots,
                         FURIOUS_RESTRICT(BitTable*) blacklist)
{
  FURIOUS_ASSERT(block_cluster->m_num_columns == 1 && "Find roots only works with single column block clusters");
  FURIOUS_ASSERT(!bitmap_is_set(&block_cluster->m_global, 0) && "block_cluster_t cannot be global");

  table_block_t* table_block = block_cluster_get_tblock(block_cluster,0);
  entity_id_t* data = (entity_id_t*)table_block->p_data;
  for(uint32_t i = 0; i < FURIOUS_TABLE_BLOCK_SIZE; ++i)
  {
    uint32_t next_entity = table_block->m_start + i;
    if(bitmap_is_set(&table_block->m_enabled, i))
    {
      uint32_t other = data[i];
      blacklist->add(next_entity);
      roots->remove(next_entity);
      if(!blacklist->exists(other))
      {
        roots->add(other);
      }
    }
  }
}

void
filter_blacklists(const DynArray<FURIOUS_RESTRICT(BitTable*)>* partial_lists,
                  FURIOUS_RESTRICT(BitTable*) current_frontier)
{
  uint32_t num_partial = partial_lists->size();
  for(uint32_t i = 0; i < num_partial; ++i)
  {
    FURIOUS_RESTRICT(BitTable*) partial_list = (*partial_lists)[i];
    bittable_difference(current_frontier, partial_list);
  }
}

void
frontiers_union(const DynArray<FURIOUS_RESTRICT(BitTable*)>* next_frontiers,
                FURIOUS_RESTRICT(BitTable*) current_frontier)
{
  uint32_t num_partial = next_frontiers->size();
  for(uint32_t i = 0; i < num_partial; ++i)
  {
    FURIOUS_RESTRICT(BitTable*) partial_frontier = (*next_frontiers)[i];
    bittable_union(current_frontier, partial_frontier);
  }
}

void
filter_bittable_exists(const BitTable* bittable, 
                       block_cluster_t* block_cluster,
                       uint32_t column)
{
  for(uint32_t i = 0; i < FURIOUS_TABLE_BLOCK_SIZE; ++i)
  {
    if(bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster_get_tblock(block_cluster, column)->p_data)[i];
      if(!bittable->exists(id))
      {
        bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

void
filter_bittable_not_exists(const BitTable* bittable, 
                           block_cluster_t* block_cluster,
                           uint32_t column)
{
  for(uint32_t i = 0; i < FURIOUS_TABLE_BLOCK_SIZE; ++i)
  {
    if(bitmap_is_set(&block_cluster->m_enabled, i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster_get_tblock(block_cluster, column)->p_data)[i];
      if(bittable->exists(id))
      {
        bitmap_unset(&block_cluster->m_enabled, i);
      }
    }
  }
}

} /* furiou */ 
