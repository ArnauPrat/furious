

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
                   const DynArray<BTree<BlockCluster>*>* hash_tables, 
                   Table** tables,
                   uint32_t num_tables)
{
  uint32_t other_block_id = (target / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE;
  uint32_t other_hash_table_index = block_get_offset(other_block_id, chunk_size, stride );
  BTree<BlockCluster>* hash_table = (*hash_tables)[other_hash_table_index];
  BlockCluster* cluster = hash_table->get(other_block_id);
  if(cluster != nullptr)
  {
    if(cluster->p_enabled->is_set(target % TABLE_BLOCK_SIZE))
    {
      for(uint32_t i = 0; i < num_tables; ++i)
      {
        Table* table_ptr = tables[i];
        void* ptr = table_ptr->alloc_component(source);
        size_t esize = cluster->get_tblock(i)->m_esize;
        void* object = &cluster->get_tblock(i)->p_data[(target%TABLE_BLOCK_SIZE)*esize];
        FURIOUS_ASSERT(object != nullptr && "Pointer to component cannot be null");
        *(uint64_t*)ptr = (uint64_t)object;
      }
    }
  }
}

void
find_roots_and_blacklist(const BlockCluster*  block_cluster, 
                         BitTable* roots,
                         BitTable* blacklist)
{
  FURIOUS_ASSERT(block_cluster->m_num_columns == 1 && "Find roots only works with single column block clusters");

  TBlock* table_block = block_cluster->get_tblock(0);
  entity_id_t* data = (entity_id_t*)table_block->p_data;
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    uint32_t next_entity = table_block->m_start + i;
    if(table_block->p_enabled->is_set(i))
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
filter_blacklists(const DynArray<BitTable*>* partial_lists,
                 BitTable* current_frontier)
{
  uint32_t num_partial = partial_lists->size();
  for(uint32_t i = 0; i < num_partial; ++i)
  {
    BitTable* partial_list = (*partial_lists)[i];
    bittable_difference(current_frontier, partial_list);
  }
}

void
frontiers_union(const DynArray<BitTable*>* next_frontiers,
                BitTable* current_frontier)
{
  uint32_t num_partial = next_frontiers->size();
  for(uint32_t i = 0; i < num_partial; ++i)
  {
    BitTable* partial_frontier = (*next_frontiers)[i];
    bittable_union(current_frontier, partial_frontier);
  }
}

void
filter_bittable_exists(const BitTable* bittable, 
                       BlockCluster* block_cluster,
                       uint32_t column)
{
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(block_cluster->p_enabled->is_set(i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster->get_tblock(column)->p_data)[i];
      if(!bittable->exists(id))
      {
        block_cluster->p_enabled->unset(i);
      }
    }
  }
}

void
filter_bittable_not_exists(const BitTable* bittable, 
                       BlockCluster* block_cluster,
                       uint32_t column)
{
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(block_cluster->p_enabled->is_set(i))
    {
      entity_id_t id = ((entity_id_t*)block_cluster->get_tblock(column)->p_data)[i];
      if(bittable->exists(id))
      {
        block_cluster->p_enabled->unset(i);
      }
    }
  }
}

} /* furiou */ 
