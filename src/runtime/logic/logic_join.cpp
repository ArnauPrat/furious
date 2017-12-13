

#include "logic_join.h"

namespace furious {

LogicJoin::LogicJoin( ILogicPlanNodeSPtr left, ILogicPlanNodeSPtr right ) : 
  p_left(left),
  p_right(right)
{}

void LogicJoin::accept( LogicPlanVisitor* visitor ) { 
  visitor->visit(this); 
};

std::string LogicJoin::str() const { 
  std::stringstream ss;
  ss << "LogicJoin()";
  return ss.str();
};

uint32_t LogicJoin::num_children() const { 
  return 2; 
};

ILogicPlanNodeSPtr LogicJoin::child( uint32_t i ) const { 
  assert( i < 2 );
  if(i == 0) return p_left;
  return p_right;
};

} /* furious */ 
