
#ifndef _FURIOUS_ENTITY_INL_
#define _FURIOUS_ENTITY_INL_

#include "database.h"
#include <assert.h>
#include <utility>

namespace furious  
{

template<typename TComponent, typename...Args>
  TComponent* 
  Entity::add_component(Args&&...args) 
  {
    TableView<TComponent> table = p_database->find_or_create_table<TComponent>();
    return table.insert_component(m_id, std::forward<Args>(args)...);
  }

template<typename TComponent>
  void Entity::remove_component() 
  {
    TableView<TComponent> table = p_database->find_table<TComponent>();
    table.remove_component(m_id);
  }

template<typename TComponent> 
  TComponent* Entity::get_component() 
  {
    TableView<TComponent> table = p_database->find_table<TComponent>();
    return static_cast<TComponent*>(table.get_component(m_id));
  }

} /* furious  */ 

#endif

