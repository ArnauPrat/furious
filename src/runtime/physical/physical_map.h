

#ifndef _FURIOUS_PHYSICAL_MAP_H
#define _FURIOUS_PHYSICAL_MAP_H value

#include "physical_plan.h"
#include "../system.h"

namespace furious {

class PhysicalMap : public IPhysicalOperator {

public:

  PhysicalMap(IPhysicalOperatorSPtr input, System* system); 
  virtual ~PhysicalMap() = default;

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////

  BaseRow* next() override;

  void open() override;

  void close() override;

  virtual uint32_t num_children()  const override ;

  virtual IPhysicalOperatorSPtr  child(uint32_t i) const override;

  virtual std::string str() const override;

private:
  IPhysicalOperatorSPtr   p_input;
  System*                 p_system;
};

} /* furious */ 
#endif
