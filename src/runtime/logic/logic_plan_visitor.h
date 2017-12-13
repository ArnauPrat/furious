

#ifndef _FURIOUS_LOGIC_PLAN_VISITOR_H_
#define _FURIOUS_LOGIC_PLAN_VISITOR_H_

namespace furious {

class LogicJoin;
class LogicMap;
class LogicScan;
class LogicFilter;

class LogicPlanVisitor {
public:
  LogicPlanVisitor() = default;
  virtual ~LogicPlanVisitor() = default;

  virtual void visit(LogicJoin* logic_join) = 0;
  virtual void visit(LogicMap* logic_map) = 0;
  virtual void visit(LogicScan* logic_scan) = 0;
  virtual void visit(LogicFilter* logic_filter) = 0;
};
} /* furious */ 
#endif
