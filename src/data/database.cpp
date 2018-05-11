


#include "database.h"
#include "../common/utils.h"

namespace furious {

Database::~Database() {
  for (auto i : m_tables) {
    delete i.second;
  }
}

Table* Database::find_table(const std::string& table_name) {
  int64_t hash_value = get_table_id(table_name);
  return find_table(hash_value);
}

Table* Database::find_table(int64_t id) {
  auto it = m_tables.find(id);
  if(it != m_tables.end() ) {
    return it->second;
  }
  assert(false);
  return nullptr;
}

int64_t Database::get_table_id(const std::string& name) {
  return hash(name);
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
    it = m_tags.insert(std::make_pair(tag, bitset())).first;
  }
  if(static_cast<int32_t>(it->second.size()) < entity_id+1) {
    it->second.resize(entity_id+1);
    it->second[entity_id] = true; 
  }

}

void Database::untag_entity(int32_t entity_id, 
                          const std::string& tag) {

  auto it = m_tags.find(tag);
  if(it != m_tags.end()) {
    if(static_cast<int32_t>(it->second.size()) > entity_id) {
      it->second[entity_id] = false;
    }
  }
}

optional<const bitset&> 
Database::get_tagged_entities(const std::string& tag) {
  auto it = m_tags.find(tag);
  if(it != m_tags.end()) {
    return optional<const bitset&>(it->second);
  }
  return optional<const bitset&>{};
}
  
} /* furious */ 
