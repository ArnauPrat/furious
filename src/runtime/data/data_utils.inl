
#include "block_cluster.h"

#include <assert.h>


namespace furious
{

template <typename...TComponents>
void
gather(const BTree<DynArray<entity_id_t>>* groups, 
       const BlockCluster* cluster,
       TableView<TComponents>*...table_view)
{

  assert(cluster->m_num_elements == sizeof...(TComponents) && "Gather operation on block cluster of incorrect size");

  Table* tables[sizeof...(TComponents)] = {table_view->get_raw()...};
  
  const Bitmap* enabled = cluster->p_enabled;
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(enabled->is_set(i))
    {
      entity_id_t id = cluster->m_start + i;
      DynArray<entity_id_t>* group = groups->get(id);
      if(group != nullptr)
      {
        for(uint32_t j = 0; j < group->size(); ++j)
        {
          entity_id_t target = (*group)[j];
          copy_component_raw(cluster, 
                             i, 
                             target, 
                             tables,
                             sizeof...(TComponents));
        }
      }
    }
  }
}

  
} /* furious */ 
