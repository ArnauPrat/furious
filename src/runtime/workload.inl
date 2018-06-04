
#include "static_system.h"
#include "../common/traits.h"
#include <utility>

namespace furious {

template<typename TSystem, typename...TArgs>
  ScopeModifier Workload::add_system(TArgs&&...args) {
    auto static_system = create_static_system<TSystem>(std::forward<TArgs>(args)...);
    m_systems.push_back(SystemExecInfo{static_system});
    return ScopeModifier{this, static_system};
  }

template<typename TSystem>
  void Workload::remove_system() {
  }

} /* furious */ 
