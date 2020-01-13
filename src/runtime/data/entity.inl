
#ifndef _FURIOUS_ENTITY_INL_
#define _FURIOUS_ENTITY_INL_

#include "database.h"
#include <utility>

namespace furious  
{

template<typename TComponent, typename...Args>
  TComponent* 
  Entity::add_component(Args&&...args) 
  {
    TableView<TComponent> table = database_find_or_create_table<TComponent>(p_database);
    return table.insert_component(m_id, std::forward<Args>(args)...);
  }

template<typename TComponent>
  void Entity::remove_component() 
  {
    TableView<TComponent> table = database_find_table<TComponent>(p_database);
    table.remove_component(m_id);
  }

template<typename TComponent> 
  TComponent* Entity::get_component() 
  {
    TableView<TComponent> table = database_find_table<TComponent>(p_database);
    return static_cast<TComponent*>(table.get_component(m_id));
  }

} /* furious  */ 

#endif

