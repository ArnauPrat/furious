
#ifndef _FURIOUS_COMPILER_PRITER_H_
#define _FURIOUS_COMPILER_PRITER_H_

#include "exec_plan.h"
#include "../common/str_builder.h"
#include "../common/dyn_array.h"

namespace furious 
{
  
struct fcc_subplan_printer_t 
{
  int32_t               m_indents;
  str_builder_t         m_str_builder;
  DynArray<char>        m_offsets;
};

void
fcc_subplan_printer_init(fcc_subplan_printer_t* printer, bool add_comments = false);

void
fcc_subplan_printer_release(fcc_subplan_printer_t* printer);

void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const FccSubPlan* plan);

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_PRITER_H_ */
