

#include "transforms.h"
#include "execution_plan.h"
#include "structs.h"

namespace furious {

Operator* bootstrap_subplan(FccExecInfo* exec_info)
{
  switch(exec_info->m_operation_type)
  {
    case OperationType::E_UNKNOWN:
      assert(false);
      break;
    case OperationType::E_FOREACH:
      // * Read basic coponent types and create one scan for each of them.
      // * Create joins to create the final table to the system will operate on
      // * Create filters for with_component, without_component, with_tag,
      //  without_tag and filter predicate.
      // * Create Foreach operator
      break;
  };
  return nullptr;
}

Foreach* merge_foreach(FccContext* context,
                       const Foreach* foreach1, 
                       const Foreach* foreach2) {
  return nullptr;
}
  
} /* furious */ 
