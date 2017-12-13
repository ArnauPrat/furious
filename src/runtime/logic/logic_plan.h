


#ifndef _FURIOUS_LOGIC_PLAN_H_
#define _FURIOUS_LOGIC_PLAN_H_ 

#include "logic_plan_visitor.h"

#include <memory>
#include <vector>
#include <cassert>

namespace furious {

class ILogicPlanNode;
using ILogicPlanNodeSPtr = std::shared_ptr<ILogicPlanNode>;

/**
 * Helper template to reduce verbosity when using shared pointers
 */
template <typename T, typename... Args>
  ILogicPlanNodeSPtr MakeLogicPlanNodeSPtr(Args&&... args)  
  {
    return std::static_pointer_cast<ILogicPlanNode>(std::make_shared<T>(std::forward<Args>(args)...));
  }

/**
 * LogicPlanNode base class
 */
class ILogicPlanNode {
public:
  ILogicPlanNode() = default;
  virtual ~ILogicPlanNode(){}

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  virtual void accept( LogicPlanVisitor* visitor ) = 0;

  virtual std::string str() const = 0;

  virtual uint32_t num_children() const = 0;

  virtual ILogicPlanNodeSPtr child( uint32_t i ) const = 0;

};

/**
 * Class representing a Logic execution plan
 */
class LogicPlan {
public:
  std::vector<ILogicPlanNodeSPtr> m_roots; // The root node of the logic plan
};
} /* furious */ 
#endif
