
#ifndef _FURIOUS_COMPILER_PRITER_H_
#define _FURIOUS_COMPILER_PRITER_H_

#include "exec_plan.h"
#include "../common/str_builder.h"

// Autogen includes
#include "autogen/char_array.h"

#define _FCC_SUBPLAN_PRINTER_MAX_OFFSETS 256 // This value must be even
  
struct fcc_subplan_printer_t 
{
  int32_t               m_indents;
  fdb_str_builder_t         m_str_builder;
  char_array_t          m_offsets;
  bool                  m_c_src_string;
};

void
fcc_subplan_printer_init(fcc_subplan_printer_t* printer, 
                         bool c_src_string = false, 
                         bool add_comments = false);

void
fcc_subplan_printer_release(fcc_subplan_printer_t* printer);

void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const fcc_subplan_t* plan);

#endif /* ifndef _FURIOUS_COMPILER_PRITER_H_ */
