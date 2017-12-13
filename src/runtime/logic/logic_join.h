

#ifndef _FURIOUS_LOGIC_JOIN_H_
#define _FURIOUS_LOGIC_JOIN_H_

#include "../common.h"
#include "logic_plan.h"

#include <memory>
#include <sstream>
#include <cassert>

namespace  furious {

class LogicJoin : public ILogicPlanNode {

public:
  LogicJoin( ILogicPlanNodeSPtr left, ILogicPlanNodeSPtr right );

  virtual ~LogicJoin() = default;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  virtual void accept( LogicPlanVisitor* visitor ) override;

  virtual std::string str() const override; 

  virtual uint32_t num_children() const override; 

  virtual ILogicPlanNodeSPtr child( uint32_t i ) const override; 

  const ILogicPlanNodeSPtr p_left;
  const ILogicPlanNodeSPtr p_right;

};
} /*  furious */ 
#endif
