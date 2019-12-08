
#include "block_cluster.h"
#include "bit_table.h"

#include "../../common/platform.h"
#include "common.h"


namespace furious
{

template <typename...TComponents>
void
gather(const BlockCluster* cluster,
       const DynArray<hashtable_t*>* hash_tables, 
       uint32_t chunk_size, 
       uint32_t stride,
       TableView<TComponents>*...table_view)
{
  FURIOUS_ASSERT(cluster->m_num_columns == 1 && "Cluster passed to gather must have a single column of references");

  Table* tables[sizeof...(TComponents)] = {table_view->get_raw()...};
  
  const bitmap_t<TABLE_BLOCK_SIZE>* enabled = &cluster->m_enabled;
  const entity_id_t* ref_data = (entity_id_t*)cluster->get_tblock(0)->p_data;
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(bitmap_is_set(enabled, i))
    {
      entity_id_t id = cluster->m_start + i;
      entity_id_t target = ref_data[i];
      copy_component_ptr(chunk_size, 
                         stride, 
                         id, 
                         target, 
                         hash_tables, 
                         tables,
                         sizeof...(TComponents));
    }
  }
}

template <typename...TComponents>
void
build_block_cluster_from_refs(const BlockCluster* ref_cluster, 
                              const BitTable* current_frontier,
                              BitTable* next_frontier,
                              BlockCluster* cluster, 
                              TableView<TComponents>*...table_views)
{
  FURIOUS_ASSERT(ref_cluster->m_num_columns == 1 && "The ref_cluster should contain a single column");

  uint32_t block_id = ref_cluster->m_start / TABLE_BLOCK_SIZE;
  Table* tables[sizeof...(TComponents)] = {table_views->get_raw()...};
  for(uint32_t i = 0; i < sizeof...(TComponents); ++i)
  {
    TBlock* tblock = tables[i]->get_block(block_id);
    if(tblock != nullptr)
    {
      cluster->append(tblock);
    }
  }
  FURIOUS_ASSERT((cluster->m_num_columns == 0 || cluster->m_num_columns == sizeof...(TComponents)) && "The block should eexist in all or in no tables");

  if(cluster->m_num_columns > 0)
  {
    bitmap_nullify(&cluster->m_enabled);
    for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
    {
      uint32_t source = i + ref_cluster->m_start;
      if(bitmap_is_set(&ref_cluster->m_enabled,i))
      {
        entity_id_t* data = (entity_id_t*)ref_cluster->get_tblock(0)->p_data;
        uint32_t target = data[i];
        if(current_frontier->exists(target))
        {
          bitmap_set(&cluster->m_enabled,i);
          FURIOUS_ASSERT(!next_frontier->exists(source) && "Source should not exist");
          next_frontier->add(source);
        }
      }
    }
  }
  return;
}

} /* furious */ 
