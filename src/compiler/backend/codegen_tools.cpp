

#include "codegen_tools.h"

#include <algorithm>

namespace furious {

std::string
gen_declare_tableview(std::ostream& output,
                      const std::string& tablename)
{

  std::string table_varname = tablename+"_table";
  std::transform(table_varname.begin(), 
                 table_varname.end(), 
                 table_varname.begin(), ::tolower);

  output << "TableView<" << tablename << "> " 
              << table_varname << " = database->find_table<"  
              << tablename << ">();" << "\n";
  output << "\n";

  return table_varname;
}

std::string 
gen_declare_biterator(std::ostream& output,
                      const std::string& varname) 
{

  std::string iter_varname = "biter_" + varname;
  output << "auto " + iter_varname << " = " << varname
              << ".iterator();\n";
  return iter_varname;
}

std::string
gen_begin_biterate(std::ostream& output,
                   const std::string& iter_varname) 
{
  std::string element_varname = "next_block";
  output << "while(" << iter_varname << ".has_next())\n{\n";
  output << "BlockCluster next_block;\n";
  output << "append(&next_block, "<< iter_varname << ".next()->get_raw());\n";
  ; 
  return "next_block";
}
  
void
gen_end_biterate(std::ostream& output) 
{
  output << "}\n\n";
}

std::string
gen_declare_bhashtable(std::ostream& output) 
{
  output << "std::unordered_map<int32_t,BlockCluster> hashtable;\n";
  return "hashtable";
}

} /* furious */ 
