

#include "data_utils.h"

namespace furious
{

void
group_references(BTree<DynArray<entity_id_t>>* groups, 
                 TableView<entity_id_t>::Block* block)
{
  entity_id_t* data = block->get_data();
  for(uint32_t i = 0 ; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(block->get_enabled()->is_set(i))
    {
      uint32_t value = data[i];
      DynArray<entity_id_t>* group = groups->get(value);
      if(group == nullptr)
      {
        group = groups->insert_new(value);
      }
      group->append(block->get_start() + i);
    }
  }
}
  
} /* furiou */ 
