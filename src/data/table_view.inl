
#include <utility>
#include <cassert>

namespace furious {

template<typename TComponent>
  TableView<TComponent>::TableView( Table* table ) :
    m_table(table)
{
  assert(m_table != nullptr);
}

template<typename TComponent>
void TableView<TComponent>::clear() {
  m_table->clear();
}

template<typename TComponent>
TComponent* TableView<TComponent>::get_element(uint32_t id) const  {
  return static_cast<TComponent*>(m_table->get_element(id));
}

template<typename TComponent>
template<typename...Args>
void TableView<TComponent>::insert_element(uint32_t id, Args&&...args){
  m_table->insert_element<TComponent>(id, std::forward<Args>(args)...);
}

template<typename TComponent>
void  TableView<TComponent>::remove_element(uint32_t id){
  m_table->remove_element(id);
}

template<typename TComponent>
void  TableView<TComponent>::enable_element(uint32_t id){
  m_table->enable_element(id);
}

template<typename TComponent>
void TableView<TComponent>::disable_element(uint32_t id){
  m_table->disable_element(id);
}

template<typename TComponent>
bool TableView<TComponent>::is_enabled(uint32_t id){
 return m_table->is_enabled(id);
}

} /* furious */ 


