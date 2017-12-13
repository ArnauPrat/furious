

#include "entity.h"

namespace furious  {

entity_id_t Entity::m_next_id = 0;

Entity::Entity(entity_id_t id) : m_id(id) {
}


Entity Entity::create_entity() {
  Entity entity{m_next_id++};
  return entity;
}

void Entity::remove_entity(Entity& entity) {
}
  
} /* furious  */ 
