


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
  
} /* furious */ 
