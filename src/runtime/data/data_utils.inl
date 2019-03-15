

namespace furious
{

template <typename TComponent>
void
copy_component_raw(TableView<TComponent>* table_view, 
                   entity_id_t id, 
                   const TComponent* component )
{
  Table* table_ptr = table_view->get_raw();
  void* ptr = table_ptr->alloc_component(id);
  memcpy(ptr, component, sizeof(TComponent));
}

template <typename TComponent>
void
gather(const BTree<DynArray<entity_id_t>>* groups, 
       const typename TableView<TComponent>::Block* block,
       TableView<TComponent>* table_view)
{
  
  TComponent* data = block->get_data();
  const Bitmap* enabled = block->get_enabled();
  for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i)
  {
    if(enabled->is_set(i))
    {
      entity_id_t id = block->get_start() + i;
      DynArray<entity_id_t>* group = groups->get(id);
      if(group != nullptr)
      {
        for(uint32_t j = 0; j < group->size(); ++j)
        {
          copy_component_raw<TComponent>(table_view, id, &data[i]);
        }
      }
    }
  }
}

  
} /* furious */ 
