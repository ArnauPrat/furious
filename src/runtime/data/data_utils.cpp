

#include "data_utils.h"

namespace furious
{

void
copy_component_raw(const BlockCluster* cluster,
                   entity_id_t source,
                   entity_id_t target,
                   Table** tables,
                   uint32_t num_tables)
{
  assert(cluster->m_num_elements == num_tables && "Copy component raw operation on block cluster of incorrect size");
  for(uint32_t i = 0; i < num_tables; ++i)
  {
    Table* table_ptr = tables[i];
    void* ptr = table_ptr->alloc_component(target);
    size_t esize = cluster->m_blocks[i]->m_esize;
    void* object = &cluster->m_blocks[i]->p_data[source*esize];
    memcpy(ptr, object, esize);
  }
}

void
group_references(BTree<DynArray<entity_id_t>>* groups, 
                 TBlock* block)
{
  entity_id_t* data = (entity_id_t*)block->p_data;
  for(uint32_t i = 0 ; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(block->p_enabled->is_set(i))
    {
      uint32_t value = data[i];
      DynArray<entity_id_t>* group = groups->get(value);
      if(group == nullptr)
      {
        group = groups->insert_new(value);
      }
      group->append(block->m_start + i);
    }
  }
}
  
} /* furiou */ 
