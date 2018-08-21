


#include "context.h"

namespace furious {

Context::Context(float dt, Database* database) : 
  m_dt{dt}, 
  p_database(database) {
  }


float Context::get_dt() {
  return m_dt;
}

Database* Context::get_database() {
  return p_database;
}
}
