

#include "../memory/memory.h"
#include <utility>

namespace furious {

template<typename TComponent, typename...Args>
void  Table::insert_element(uint32_t id, Args&&...args) 
{
  new (alloc_element(id)) TComponent{std::forward<Args>(args)...};
}

} /* furious  */ 
