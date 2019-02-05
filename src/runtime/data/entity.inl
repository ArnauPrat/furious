
#include "database.h"
#include <assert.h>
#include <utility>

namespace furious  {

template<typename TComponent, typename...Args>
  void Entity::add_component(const std::string& component,
                             Args&&...args) 
  {
    TableView<TComponent> table = p_database->find_or_create_table<TComponent>(component);
    table.insert_element(m_id, std::forward<Args>(args)...);
  }

template<typename TComponent>
  void Entity::remove_component(const std::string& component) 
  {
    TableView<TComponent> table = p_database->find_table<TComponent>(component);
    table.remove_element(m_id);
  }

template<typename TComponent> 
  TComponent* Entity::get_component(const std::string& component) 
  {
    TableView<TComponent> table = p_database->find_table<TComponent>(component);
    return static_cast<TComponent*>(table.get_element(m_id));
  }

} /* furious  */ 

