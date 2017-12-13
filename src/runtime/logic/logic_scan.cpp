
#include "logic_scan.h"

namespace furious {

LogicScan::LogicScan(const std::string& table) : 
  m_table(table) {
  }

void LogicScan::accept( LogicPlanVisitor* visitor ) { 
  visitor->visit(this);
};

std::string LogicScan::str() const { 
  std::stringstream ss;
  ss << "LogicScan(" << m_table << ")";
  return ss.str();
};

uint32_t LogicScan::num_children() const { 
  return 0; 
};

ILogicPlanNodeSPtr LogicScan::child( uint32_t i ) const { 
  assert(false);
  return nullptr;
};

} /* furious */ 
