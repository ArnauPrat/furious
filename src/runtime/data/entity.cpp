
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

Entity::Entity(database_t* database,
               entity_id_t id) :
m_id(id),
p_database(database)
{
}

Entity::Entity(database_t* database) : 
  m_id(database_get_next_entity_id(database)),
  p_database(database)
{
}


Entity 
Entity::create_entity(database_t* database) 
{
  Entity entity(database);
  return entity;
}

void 
Entity::remove_entity(Entity entity) 
{
  database_clear_entity(entity.p_database, 
                        entity.m_id);
  entity.m_id = FURIOUS_INVALID_ID;
}

void 
Entity::add_tag(const char* tag) 
{
  database_tag_entity(p_database, m_id, tag);
}

void 
Entity::remove_tag(const char* tag) 
{
  database_untag_entity(p_database, m_id, tag);
}

bool 
Entity::has_tag(const char* tag)
{
  const BitTable* bit_table = database_get_tagged_entities(p_database, tag);
  return bit_table->exists(m_id);
}

void
Entity::add_reference(const char* ref_name,
                      Entity other)
{
  database_add_reference(p_database, 
                         ref_name, 
                         m_id, 
                         other.m_id);
}

void
Entity::remove_reference(const char* ref_name)
{
  database_remove_reference(p_database, 
                            ref_name, 
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
  return Entity(p_database,FURIOUS_INVALID_ID);
}

database_t* 
Entity::get_database() 
{
  return p_database;
}

bool
Entity::is_valid()
{
  return m_id != FURIOUS_INVALID_ID;
}

Entity 
create_entity(database_t* database) 
{
 return Entity::create_entity(database);
}

void 
destroy_entity(Entity entity) 
{
  Entity::remove_entity(entity);
}
  
} /* furious  */ 
