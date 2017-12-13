

#include <cassert>
#include <utility>
#include "memory/memory.h"

namespace furious {

extern uint8_t bitmap_masks[8];

template<typename TComponent, typename...Args>
  void  Table::insert_element(uint32_t id, Args&&...args) {
    new (alloc_element(id)) TComponent{std::forward<Args>(args)...};
  }

} /* furious  */ 
