

#ifndef _FURIOUS_LOGIC_MAP_H
#define _FURIOUS_LOGIC_MAP_H

#include "../common.h"
#include "../system.h"
#include "logic_plan.h"

#include <sstream>
#include <cassert>

namespace furious {

class LogicMap : public ILogicPlanNode {

public:
  LogicMap( SystemId system, ILogicPlanNodeSPtr table );
  virtual ~LogicMap() = default;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  virtual void accept( LogicPlanVisitor* visitor ) override; 

  virtual std::string str() const override;

  virtual uint32_t num_children() const override;

  virtual ILogicPlanNodeSPtr child( uint32_t i ) const override; 

  const SystemId            m_system;
  const ILogicPlanNodeSPtr  p_table;

};
} /* furious */ 
#endif
