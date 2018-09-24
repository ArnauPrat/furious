
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include <string>
#include <ostream>

namespace furious 
{

std::string 
gen_declare_tableview(std::ostream& output,
                      const std::string& tablename);

std::string 
gen_declare_biterator(std::ostream& output,
                      const std::string& varname);

std::string
gen_begin_biterate(std::ostream& output,
                   const std::string& iter_varname);

void
gen_end_biterate(std::ostream& output);


std::string
gen_declare_bhashtable(std::ostream& output);
  
} /* furious */ 

#endif
