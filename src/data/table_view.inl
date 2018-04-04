
#include <utility>
#include <cassert>

namespace furious {

template<typename TComponent>
  TableView<TComponent>::Iterator::Iterator(Table::Iterator iter ) : m_table_it(iter),
  p_block_it(nullptr) {
  }

template<typename TComponent>
  TableView<TComponent>::Iterator::~Iterator() {
  }

template<typename TComponent>
bool TableView<TComponent>::Iterator::has_next() const {
  return p_block_it.has_next() || (!p_block_it.has_next() && m_table_it.has_next());
}

template<typename TComponent>
typename TableView<TComponent>::Row TableView<TComponent>::Iterator::next() {
  if(!p_block_it.has_next()) {
    if(m_table_it.has_next()) {
      p_block_it.reset(m_table_it.next());
    }
  }
  TRow row = p_block_it.next();
  return Row{row.m_id, reinterpret_cast<TComponent*>(row.p_data), row.m_enabled};
}


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
TComponent* TableView<TComponent>::get_element(uint32_t id) const  {
  return static_cast<TComponent*>(p_table->get_element(id));
}

template<typename TComponent>
template<typename...Args>
void TableView<TComponent>::insert_element(uint32_t id, Args&&...args){
  p_table->insert_element<TComponent>(id, std::forward<Args>(args)...);
}

template<typename TComponent>
void  TableView<TComponent>::remove_element(uint32_t id){
  p_table->remove_element(id);
}

template<typename TComponent>
void  TableView<TComponent>::enable_element(uint32_t id){
  p_table->enable_element(id);
}

template<typename TComponent>
void TableView<TComponent>::disable_element(uint32_t id){
  p_table->disable_element(id);
}

template<typename TComponent>
bool TableView<TComponent>::is_enabled(uint32_t id){
 return p_table->is_enabled(id);
}


template<typename TComponent>
size_t TableView<TComponent>::size() const {
  return p_table->size();
}

template<typename TComponent>
typename TableView<TComponent>::Iterator TableView<TComponent>::iterator() {
  return Iterator(p_table->iterator());
}

} /* furious */ 


