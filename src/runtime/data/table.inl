

#include <cassert>
#include <utility>
#include "../memory/memory.h"

namespace furious {

template<typename TComponent, typename...Args>
  void  Table::insert_element(int32_t id, Args&&...args) {
    new (alloc_element(id)) TComponent{std::forward<Args>(args)...};
  }

} /* furious  */ 
