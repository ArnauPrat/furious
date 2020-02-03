
#include <utility>
#include <assert.h>

namespace furious 
{

template<typename TComponent>
TableView<TComponent>::Block::Block(table_block_t* block) : p_table_block_t{block} 
{

}

template<typename TComponent>
TComponent* 
TableView<TComponent>::Block::get_data() const 
{
  return reinterpret_cast<TComponent*>(p_table_block_t->p_data);
}

template<typename TComponent>
size_t 
TableView<TComponent>::Block::get_num_components() const 
{
  return p_table_block_t->m_num_components;
}

template<typename TComponent>
entity_id_t TableView<TComponent>::Block::get_start() const 
{
  return p_table_block_t->m_start;
}

template<typename TComponent>
size_t TableView<TComponent>::Block::get_size() const 
{
  return FURIOUS_TABLE_BLOCK_SIZE;
}

template<typename TComponent>
const bitmap_t*   
TableView<TComponent>::Block::get_enabled() const 
{
  return &p_table_block_t->m_enabled;
}

template<typename TComponent>
table_block_t* 
TableView<TComponent>::Block::get_raw() const 
{
  return p_table_block_t;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


template<typename TComponent>
TableView<TComponent>::BlockIterator::BlockIterator(table_iter_t iter ) : 
m_iterator(iter) 
{
}

template<typename TComponent>
bool 
TableView<TComponent>::BlockIterator::has_next() 
{
  return table_iter_has_next(&m_iterator);
}

template<typename TComponent>
typename TableView<TComponent>::Block 
TableView<TComponent>::BlockIterator::next() 
{
  return Block{table_iter_next(&m_iterator)};
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
  TableView<TComponent>::TableView( table_t* table ) :
    p_table(table)
{
  assert(p_table != nullptr);
}

template<typename TComponent>
void 
TableView<TComponent>::clear() 
{
  table_clear(p_table);
}

template<typename TComponent>
TComponent* 
TableView<TComponent>::get_component(entity_id_t id) const  
{
  return static_cast<TComponent*>(table_get_component(p_table, id));
}

template<typename TComponent>
template<typename...Args>
TComponent* 
TableView<TComponent>::insert_component(entity_id_t id, Args&&...args)
{
 return new (table_alloc_component(p_table, id)) TComponent{std::forward<Args>(args)...};
}

template<typename TComponent>
void  TableView<TComponent>::remove_component(entity_id_t id)
{
  table_dealloc_component(p_table, id);
}

template<typename TComponent>
void  TableView<TComponent>::enable_component(entity_id_t id)
{
  table_enable_component(p_table, id);
}

template<typename TComponent>
void TableView<TComponent>::disable_component(entity_id_t id)
{
  table_disable_component(p_table, id);
}

template<typename TComponent>
bool TableView<TComponent>::is_enabled(entity_id_t id)
{
 return table_is_enabled(p_table, id);
}


template<typename TComponent>
size_t TableView<TComponent>::size() const 
{
  return table_size(p_table);
}

template<typename TComponent>
typename TableView<TComponent>::BlockIterator TableView<TComponent>::iterator() 
{
  return BlockIterator{table_iter_create(p_table)};
}

template<typename TComponent>
typename TableView<TComponent>::BlockIterator TableView<TComponent>::iterator(uint32_t chunk_size, uint32_t offset, uint32_t stride) 
{
  return BlockIterator{table_iter_create(p_table, chunk_size, offset, stride)};
}

template <typename TComponent>
table_t*
TableView<TComponent>::get_raw()
{
  return p_table;
}

template <typename TComponent>
TableView<TComponent>::BlockIterator::~BlockIterator()
{
  table_iter_destroy(&m_iterator);
}

} /* furious */ 


