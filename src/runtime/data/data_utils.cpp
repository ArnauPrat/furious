

#include "data_utils.h"
#include "bit_table.h"

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

void
find_roots(const BTree<DynArray<entity_id_t>>* groups, 
           BitTable* roots)
{
  BTree<DynArray<entity_id_t>>::Iterator it = groups->iterator();
  while(it.has_next())
  {
    entity_id_t next_id = it.next().m_key;
    roots->add(next_id);
  }

  it = groups->iterator();
  while(it.has_next())
  {
    auto entry = it.next();
    DynArray<entity_id_t>& group = *entry.p_value;
    uint32_t size = group.size();
    for(uint32_t i = 0; i < size; ++i)
    {
      roots->remove(group[i]);
    }
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
      entity_id_t id = ((entity_id_t*)block_cluster->m_blocks[column]->p_data)[i];
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
      entity_id_t id = ((entity_id_t*)block_cluster->m_blocks[column]->p_data)[i];
      if(bittable->exists(id))
      {
        block_cluster->p_enabled->unset(i);
      }
    }
  }
}
  
} /* furiou */ 
