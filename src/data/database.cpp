


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
  uint64_t hash_value = hash(table_name);
  auto it = m_tables.find(hash_value);
  if(it != m_tables.end() ) {
    return it->second;
  }
  assert(false);
}

void Database::clear() {
  for (auto i : m_tables) {
    delete i.second;
  }
  m_tables.clear();
}
  
} /* furious */ 
