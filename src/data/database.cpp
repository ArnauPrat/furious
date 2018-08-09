


#include "database.h"
#include "../common/utils.h"

namespace furious {

Database::~Database() {
  for (auto i : m_tables) {
    delete i.second;
  }

  for (auto i : m_tags) {
    delete i.second;
  }
}

void Database::clear() {
  for (auto i : m_tables) {
    delete i.second;
  }
  m_tables.clear();
}

int32_t Database::get_next_entity_id() {
  int32_t next_id = m_next_entity_id;
  m_next_entity_id++;
  return next_id;
}

void Database::clear_element(int32_t id) {
  for(auto table : m_tables) {
    table.second->remove_element(id);
  }
}

void Database::tag_entity(int32_t entity_id, 
                          const std::string& tag) {

  auto it = m_tags.find(tag);
  if(it == m_tags.end()) {
    it = m_tags.insert(std::make_pair(tag, new bitset())).first;
  }
  if(static_cast<int32_t>(it->second->size()) < entity_id+1) {
    it->second->resize(entity_id+1);
    (*(it->second))[entity_id] = true; 
  }

}

void Database::untag_entity(int32_t entity_id, 
                          const std::string& tag) {

  auto it = m_tags.find(tag);
  if(it != m_tags.end()) {
    if(static_cast<int32_t>(it->second->size()) > entity_id) {
    (*(it->second))[entity_id] = false; 
    }
  }
}

optional<const bitset*> 
Database::get_tagged_entities(const std::string& tag) {
  auto it = m_tags.find(tag);
  if(it != m_tags.end()) {
    return optional<const bitset*>(it->second);
  }
  return optional<const bitset*>{};
}

void Database::add_reference( const std::string& type, 
                              int32_t tail, 
                              int32_t head) {

  /*auto it = m_references.find(type);
  if (it == m_references.end()) {
    int64_t hash_value = get_table_id(type);
    auto table = new Table(type, hash_value, sizeof(int32_t), &destructor<int32_t>);
    it = m_references.insert( std::make_pair(type, table)).first;
  }

  it->second->insert_element<int32_t>(tail, head);
  */

}

void Database::remove_reference( const std::string& type, 
                                 int32_t tail, 
                                 int32_t head) {

  /*auto it = m_references.find(type);
  if (it != m_references.end()) {
    it->second->remove_element(tail);
  }*/

}

  
} /* furious */ 
