

#include "system.h"

namespace furious {

System::System(int32_t id) : 
  m_id{id} {

  }

int32_t System::get_id() {
  return m_id;
}
  
} /* furious */ 
