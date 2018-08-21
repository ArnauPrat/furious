

#include "entity.h"

namespace furious  {

entity_id_t Entity::m_next_id = 0;


Entity::Entity(Database* database) : 
  m_id(database->get_next_entity_id()),
  p_database(database)
{
}


Entity Entity::create_entity(Database* database) {
  Entity entity{database};
  return entity;
}

void Entity::remove_entity(Entity* entity) {
  entity->get_database()->clear_element(entity->m_id);
  entity->m_id = FURIOUS_INVALID_ID;
}

void Entity::add_tag(const std::string& tag) {
  p_database->tag_entity(m_id, tag);
}

void Entity::remove_tag(const std::string& tag) {
  p_database->untag_entity(m_id, tag);
}

Database* Entity::get_database() {
  return p_database;
}
  
} /* furious  */ 
