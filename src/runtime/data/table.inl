

#include "../memory/memory.h"
#include <utility>

namespace furious {

template<typename TComponent, typename...Args>
void  Table::insert_component(uint32_t id, Args&&...args) 
{
  new (alloc_component(id)) TComponent{std::forward<Args>(args)...};
}

} /* furious  */ 
