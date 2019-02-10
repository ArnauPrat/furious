
#include "entity.h"
#include "bit_table.h"

namespace furious  {

Entity::Entity(Database* database) : 
  m_id(database->get_next_entity_id()),
  p_database(database)
{
}


Entity 
Entity::create_entity(Database* database) 
{
  Entity entity(database);
  return entity;
}

void 
Entity::remove_entity(Entity entity) 
{
  entity.get_database()->clear_element(entity.m_id);
  entity.m_id = FURIOUS_INVALID_ID;
}

void 
Entity::add_tag(const std::string& tag) 
{
  p_database->tag_entity(m_id, tag);
}

void 
Entity::remove_tag(const std::string& tag) 
{
  p_database->untag_entity(m_id, tag);
}

bool 
Entity::has_tag(const std::string& tag)
{
  const BitTable* bit_table = p_database->get_tagged_entities(tag);
  return bit_table->exists(m_id);
}

Database* Entity::get_database() 
{
  return p_database;
}

Entity 
create_entity(Database* database) 
{
 return Entity::create_entity(database);
}

void 
destroy_entity(Entity entity) 
{
  Entity::remove_entity(entity);
}
  
} /* furious  */ 
