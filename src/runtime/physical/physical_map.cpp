
#include "physical_map.h"

#include <cassert>
#include <sstream>

namespace furious {

PhysicalMap::PhysicalMap( IPhysicalOperatorSPtr input, System* system ) :
  p_input(input),
  p_system(system)
  {

  }

BaseRow* PhysicalMap::next() {
  BaseRow* next_row = p_input->next();
  while(next_row != nullptr && !next_row->m_enabled) {
    next_row = p_input->next();
  }
  if(next_row != nullptr) {
    std::vector<void*> components;
    for(uint32_t i = 0; i < next_row->num_columns(); ++i) {
      components.push_back(next_row->column(i));
    }
    p_system->apply(components);
  }
  return next_row;
}

void PhysicalMap::open() {
  p_input->open();

}

void PhysicalMap::close() {
  p_input->close();
}

uint32_t PhysicalMap::num_children()  const  {
  return 1;
}

IPhysicalOperatorSPtr  PhysicalMap::child(uint32_t i ) const {
  assert(i == 0);
  return p_input;
}

std::string PhysicalMap::str() const  {
  std::stringstream ss;
  ss << "PhysicalMap(" << p_system->m_id << ")";
  return ss.str();
}

} /* furious */ 
