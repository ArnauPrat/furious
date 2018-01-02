
#include "database.h"

namespace furious {

template <typename TComponent>
  void Context::enable_component(uint32_t id) {
    m_to_enable.emplace_back(id, Database::get_instance()->get_table_id<TComponent>());
  }

template<typename TComponent>
  void Context::disable_component(uint32_t id) {
    m_to_disable.emplace_back(id, Database::get_instance()->get_table_id<TComponent>());
  }

  
} /* furious */ 
