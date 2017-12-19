


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
  for( auto it : m_tables ) {
    if(it.second->table_name() == table_name ) {
      return it.second;
    }
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
