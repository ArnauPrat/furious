

#include "physical_filter.h"
#include <cassert>

namespace furious {

PhysicalFilter::PhysicalFilter( IPhysicalOperatorSPtr input ) :
  p_input(input)
{

}

BaseRow* PhysicalFilter::next() {
  BaseRow* next_row = p_input->next();
  while(next_row != nullptr && !next_row->m_enabled) {
    next_row = p_input->next();
  }
  return next_row;
}

void PhysicalFilter::open() {
  p_input->open();

}

void PhysicalFilter::close() {
  p_input->close();
}

uint32_t PhysicalFilter::num_children()  const  {
  return 1;
}

IPhysicalOperatorSPtr  PhysicalFilter::child(uint32_t i ) const {
  assert(i == 0);
  return p_input;
}

std::string PhysicalFilter::str() const  {
  return "PhysicalFilter()";
}

} /* furious */ 
