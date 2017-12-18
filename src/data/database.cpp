


#include "database.h"

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
  assert(m_tables.find(table_name) != m_tables.end());
  auto table = m_tables.find(table_name);
  if(table == m_tables.end()) {
    return nullptr;
  }
  return table->second;
}

void Database::clear() {
  for (auto i : m_tables) {
    delete i.second;
  }
  m_tables.clear();
}
  
} /* furious */ 
