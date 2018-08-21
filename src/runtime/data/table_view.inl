
#include <utility>
#include <cassert>

namespace furious {

template<typename TComponent>
TableView<TComponent>::Block::Block(TBlock* block) : p_tblock{block} {

}

template<typename TComponent>
  TComponent* TableView<TComponent>::Block::get_data() const {
    return reinterpret_cast<TComponent*>(p_tblock->p_data);
  }

template<typename TComponent>
  size_t TableView<TComponent>::Block::get_num_elements() const {
    return p_tblock->m_num_elements;
  }

template<typename TComponent>
  size_t TableView<TComponent>::Block::get_size() const {
    return TABLE_BLOCK_SIZE;
  }

template<typename TComponent>
const std::bitset<TABLE_BLOCK_SIZE>&   TableView<TComponent>::Block::get_enabled() const {
  return p_tblock->m_enabled;
}
  

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


template<typename TComponent>
  TableView<TComponent>::BlockIterator::BlockIterator(Table::Iterator iter ) : m_iterator(iter) {
  }

template<typename TComponent>
bool TableView<TComponent>::BlockIterator::has_next() const {
  return m_iterator.has_next();
}

template<typename TComponent>
typename TableView<TComponent>::Block TableView<TComponent>::BlockIterator::next() {
  return Block{m_iterator.next()};
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename TComponent>
  TableView<TComponent>::TableView( Table* table ) :
    p_table(table)
{
  assert(p_table != nullptr);
}

template<typename TComponent>
void TableView<TComponent>::clear() {
  p_table->clear();
}

template<typename TComponent>
TComponent* TableView<TComponent>::get_element(int32_t id) const  {
  return static_cast<TComponent*>(p_table->get_element(id));
}

template<typename TComponent>
template<typename...Args>
void TableView<TComponent>::insert_element(int32_t id, Args&&...args){
  p_table->insert_element<TComponent>(id, std::forward<Args>(args)...);
}

template<typename TComponent>
void  TableView<TComponent>::remove_element(int32_t id){
  p_table->remove_element(id);
}

template<typename TComponent>
void  TableView<TComponent>::enable_element(int32_t id){
  p_table->enable_element(id);
}

template<typename TComponent>
void TableView<TComponent>::disable_element(int32_t id){
  p_table->disable_element(id);
}

template<typename TComponent>
bool TableView<TComponent>::is_enabled(int32_t id){
 return p_table->is_enabled(id);
}


template<typename TComponent>
size_t TableView<TComponent>::size() const {
  return p_table->size();
}

template<typename TComponent>
typename TableView<TComponent>::BlockIterator TableView<TComponent>::iterator() {
  return BlockIterator{p_table->iterator()};
}

} /* furious */ 


