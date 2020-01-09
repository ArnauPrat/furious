
#include "macros.h"
#include "entity.h"
#include "bit_table.h"

namespace furious  
{

Entity::Entity() :
m_id(FURIOUS_INVALID_ID),
p_database(nullptr)
{
}

Entity::Entity(Database* database,
               entity_id_t id) :
m_id(id),
p_database(database)
{
}

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
  entity.get_database()->clear_entity(entity.m_id);
  entity.m_id = FURIOUS_INVALID_ID;
}

void 
Entity::add_tag(const char* tag) 
{
  p_database->tag_entity(m_id, tag);
}

void 
Entity::remove_tag(const char* tag) 
{
  p_database->untag_entity(m_id, tag);
}

bool 
Entity::has_tag(const char* tag)
{
  const BitTable* bit_table = p_database->get_tagged_entities(tag);
  return bit_table->exists(m_id);
}

void
Entity::add_reference(const char* ref_name,
                      Entity other)
{
  p_database->add_reference(ref_name, 
                            m_id, 
                            other.m_id);
}

void
Entity::remove_reference(const char* ref_name)
{
  p_database->remove_reference(ref_name, 
                               m_id);
}

Entity
Entity::get_reference(const char* ref_name)
{
  TableView<uint32_t> ref_table = FURIOUS_FIND_OR_CREATE_REF_TABLE(p_database, ref_name);
  uint32_t* other = ref_table.get_component(m_id);
  if(other != nullptr)
  {
    return Entity(p_database,*other);
  }
  return Entity(p_database,_FURIOUS_INVALID_ENTITY_ID);
}

Database* 
Entity::get_database() 
{
  return p_database;
}

bool
Entity::is_valid()
{
  return m_id != _FURIOUS_INVALID_ENTITY_ID;
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
