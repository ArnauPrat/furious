

#ifndef _FURIOUS_PHYSICAL_SCAN_H
#define _FURIOUS_PHYSICAL_SCAN_H

#include "physical_plan.h"
#include "../table.h"
#include "../common.h"

namespace furious {

class PhysicalScan : public IPhysicalOperator {

public:

  PhysicalScan(Table* table); 
  virtual ~PhysicalScan() = default;

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
  Table*              p_table;
  Table::iterator     m_iterator;
};

} /* furious */ 
#endif
