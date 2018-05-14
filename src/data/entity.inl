
#include "database.h"
#include <cassert>
#include <utility>

namespace furious  {

template<typename TComponent, typename...Args>
  void Entity::add_component(Args&&...args) {

    if(!p_database->exists_table<TComponent>()) {
      p_database->add_table<TComponent>();
    }

    TableView<TComponent> table = p_database->find_table<TComponent>();
    table.insert_element(m_id, std::forward<Args>(args)...);
  }

template<typename TComponent>
  void Entity::remove_component() {

    TableView<TComponent> table = p_database->find_table<TComponent>();
    table.remove_element(m_id);
  }

template<typename TComponent> 
  TComponent* Entity::get_component() {
    TableView<TComponent> table = p_database->find_table<TComponent>();
    return static_cast<TComponent*>(table.get_element(m_id));
  }

} /* furious  */ 

