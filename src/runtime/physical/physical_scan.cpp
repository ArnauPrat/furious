

#include "../database.h"
#include "physical_scan.h"

#include <cassert>
#include <sstream>

namespace furious {

PhysicalScan::PhysicalScan(Table* table) :
  p_table(table),
  m_iterator(p_table->begin()){}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

BaseRow* PhysicalScan::next() {
  BaseRow* ret = (*m_iterator);
  ++m_iterator;
  return ret;
}

void PhysicalScan::open() {
}

void PhysicalScan::close() {

}

uint32_t PhysicalScan::num_children()  const  {
  return 0;
}

IPhysicalOperatorSPtr  PhysicalScan::child(uint32_t i ) const {
  assert(false);
  return nullptr;
}

std::string PhysicalScan::str() const  {
  std::stringstream ss;
  ss << "PhysicalScan(" << p_table->table_name() << ")";
  return ss.str();
}

} /* furious */ 
