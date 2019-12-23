

#include "../../common/platform.h"
#include "../../common/dyn_array.h"
#include "common.h"
#include "block_cluster.h"
#include "bit_table.h"
#include "table_view.h"


namespace furious
{

template <typename...TComponents>
void
gather(block_cluster_t* cluster,
       const DynArray<FURIOUS_RESTRICT(hashtable_t*)>* hash_tables, 
       uint32_t chunk_size, 
       uint32_t stride,
       TableView<TComponents>*...table_view)
{
  FURIOUS_ASSERT(cluster->m_num_columns == 1 && "Cluster passed to gather must have a single column of references");

  FURIOUS_RESTRICT(Table*) tables[sizeof...(TComponents)] = {table_view->get_raw()...};
  
  const bitmap_t* enabled = &cluster->m_enabled;
  const entity_id_t* ref_data = (entity_id_t*)block_cluster_get_tblock(cluster,0)->p_data;
  for(uint32_t i = 0; i < FURIOUS_TABLE_BLOCK_SIZE; ++i)
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
build_block_cluster_from_refs(FURIOUS_RESTRICT(block_cluster_t*) ref_cluster, 
                              FURIOUS_RESTRICT(const BitTable*) current_frontier,
                              FURIOUS_RESTRICT(BitTable*) next_frontier,
                              FURIOUS_RESTRICT(block_cluster_t*) cluster, 
                              TableView<TComponents>*...table_views)
{
  FURIOUS_ASSERT(ref_cluster->m_num_columns == 1 && "The ref_cluster should contain a single column");

  uint32_t block_id = ref_cluster->m_start / FURIOUS_TABLE_BLOCK_SIZE;
  FURIOUS_RESTRICT(Table*) tables[sizeof...(TComponents)] = {table_views->get_raw()...};
  for(uint32_t i = 0; i < sizeof...(TComponents); ++i)
  {
    FURIOUS_RESTRICT(TBlock*) tblock = tables[i]->get_block(block_id);
    if(tblock != nullptr)
    {
      block_cluster_append(cluster, tblock);
    }
  }
  FURIOUS_ASSERT((cluster->m_num_columns == 0 || cluster->m_num_columns == sizeof...(TComponents)) && "The block should eexist in all or in no tables");

  if(cluster->m_num_columns > 0)
  {
    entity_id_t* data = (entity_id_t*)block_cluster_get_tblock(ref_cluster,0)->p_data;
    bitmap_nullify(&cluster->m_enabled);
    for(uint32_t i = 0; i < FURIOUS_TABLE_BLOCK_SIZE; ++i)
    {
      uint32_t source = i + ref_cluster->m_start;
      if(bitmap_is_set(&ref_cluster->m_enabled,i))
      {
        entity_id_t target = data[i];
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
