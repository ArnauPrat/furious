
#include <utility>
#include <assert.h>

namespace furious 
{

template<typename TComponent>
TableView<TComponent>::Block::Block(TBlock* block) : p_tblock{block} 
{

}

template<typename TComponent>
TComponent* 
TableView<TComponent>::Block::get_data() const 
{
  return reinterpret_cast<TComponent*>(p_tblock->p_data);
}

template<typename TComponent>
size_t 
TableView<TComponent>::Block::get_num_components() const 
{
  return p_tblock->m_num_components;
}

template<typename TComponent>
entity_id_t TableView<TComponent>::Block::get_start() const 
{
  return p_tblock->m_start;
}

template<typename TComponent>
size_t TableView<TComponent>::Block::get_size() const 
{
  return TABLE_BLOCK_SIZE;
}

template<typename TComponent>
const Bitmap*   
TableView<TComponent>::Block::get_enabled() const 
{
  return p_tblock->p_enabled;
}

template<typename TComponent>
TBlock* 
TableView<TComponent>::Block::get_raw() const 
{
  return p_tblock;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


template<typename TComponent>
TableView<TComponent>::BlockIterator::BlockIterator(Table::Iterator iter ) : 
m_iterator(iter) 
{
}

template<typename TComponent>
bool 
TableView<TComponent>::BlockIterator::has_next() const 
{
  return m_iterator.has_next();
}

template<typename TComponent>
typename TableView<TComponent>::Block 
TableView<TComponent>::BlockIterator::next() 
{
  return Block{m_iterator.next()};
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename TComponent>
  TableView<TComponent>::TableView() :
    p_table(nullptr)
{
}

template<typename TComponent>
  TableView<TComponent>::TableView( Table* table ) :
    p_table(table)
{
  assert(p_table != nullptr);
}

template<typename TComponent>
void 
TableView<TComponent>::clear() 
{
  p_table->clear();
}

template<typename TComponent>
TComponent* 
TableView<TComponent>::get_component(entity_id_t id) const  
{
  return static_cast<TComponent*>(p_table->get_component(id));
}

template<typename TComponent>
template<typename...Args>
void 
TableView<TComponent>::insert_component(entity_id_t id, Args&&...args)
{
  new (p_table->alloc_component(id)) TComponent{std::forward<Args>(args)...};
}

template<typename TComponent>
void  TableView<TComponent>::remove_component(entity_id_t id)
{
  p_table->dealloc_and_destroy_component(id);
}

template<typename TComponent>
void  TableView<TComponent>::enable_component(entity_id_t id)
{
  p_table->enable_component(id);
}

template<typename TComponent>
void TableView<TComponent>::disable_component(entity_id_t id)
{
  p_table->disable_component(id);
}

template<typename TComponent>
bool TableView<TComponent>::is_enabled(entity_id_t id)
{
 return p_table->is_enabled(id);
}


template<typename TComponent>
size_t TableView<TComponent>::size() const 
{
  return p_table->size();
}

template<typename TComponent>
typename TableView<TComponent>::BlockIterator TableView<TComponent>::iterator() 
{
  return BlockIterator{p_table->iterator()};
}

template <typename TComponent>
Table*
TableView<TComponent>::get_raw()
{
  return p_table;
}

} /* furious */ 


