
#include "database.h"

namespace furious {

template <typename TComponent>
  void Context::enable_component(int32_t id) {
    m_to_enable.emplace_back(id, p_database->get_table_id<TComponent>());
  }

template<typename TComponent>
  void Context::disable_component(int32_t id) {
    m_to_disable.emplace_back(id, p_database->get_table_id<TComponent>());
  }

  
} /* furious */ 
