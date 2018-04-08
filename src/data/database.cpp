


#include "database.h"
#include "../common/utils.h"

namespace furious {

Database::~Database() {
  for (auto i : m_tables) {
    delete i.second;
  }
}

Database* Database::get_instance() {
  static Database instance;
  return &instance;
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
  
} /* furious */ 
