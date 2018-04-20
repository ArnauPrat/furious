
#include "static_system.h"
#include "../common/reflection.h"
#include <utility>

namespace furious {

template<typename TSystem, typename...TArgs>
  void Workload::add_system(TArgs&&...args) {
    auto static_system = create_static_system<TSystem>(std::forward<TArgs>(args)...);
    m_systems.insert(std::make_pair(type_name<TSystem>(), static_system));
  }

template<typename TSystem>
  void Workload::remove_system() {
  }

} /* furious */ 
