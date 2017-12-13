

#ifndef _FURIOUS_LOGIC_FILTER_H
#define _FURIOUS_LOGIC_FILTER_H 

#include "../common.h"
#include "logic_plan.h"

#include <sstream>

namespace  furious {

class LogicFilter : public ILogicPlanNode {

public:
  LogicFilter(ILogicPlanNodeSPtr table);
  virtual ~LogicFilter() = default;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  virtual void accept( LogicPlanVisitor* visitor ) override;

  virtual std::string str() const override;

  virtual uint32_t num_children() const override;

  virtual ILogicPlanNodeSPtr child( uint32_t i ) const override; 

  const ILogicPlanNodeSPtr p_table;

};

} /*  furious */ 
#endif
