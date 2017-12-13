


#ifndef _FURIOUS_PHYSICAL_PLAN_GENERATOR_H_
#define _FURIOUS_PHYSICAL_PLAN_GENERATOR_H_

#include "logic/logic_plan_visitor.h"
#include "physical/physical_plan.h"

namespace furious {
class PhysicalPlanGenerator : public LogicPlanVisitor {
public:
  PhysicalPlanGenerator() = default;
  virtual ~PhysicalPlanGenerator() = default;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  virtual void  visit(LogicJoin* logic_join) override;
  virtual void  visit(LogicMap* logic_map) override;
  virtual void  visit(LogicScan* logic_scan) override;
  virtual void  visit(LogicFilter* logic_filter) override;

  IPhysicalOperatorSPtr get_result(); 

private:

  IPhysicalOperatorSPtr p_result;

};
} /* furious */ 

#endif
