

#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"

#include <clang/AST/PrettyPrinter.h>
#include <string>

using namespace clang;

namespace furious 
{

ProduceVisitor::ProduceVisitor(std::stringstream& output_ss) :
m_output_ss{output_ss}
{
}

void 
ProduceVisitor::visit(const Foreach* foreach)
{
  ProduceVisitor produce{m_output_ss};
  foreach->p_child->accept(&produce);
}

void 
ProduceVisitor::visit(const Scan* scan)
{
  std::string ctype = get_type_name(scan->m_component);
  std::string q_ctype = get_qualified_type_name(scan->m_component);
  std::string base_name = ctype;

  std::transform(base_name.begin(), 
                 base_name.end(), 
                 base_name.begin(), ::tolower);

  std::string table_varname = base_name+"_table";
  std::string iter_varname = base_name+"_iter";
  std::string block_varname = base_name+"_block"; 

  m_output_ss << "auto " + iter_varname << " = " << table_varname << ".iterator();\n";
  m_output_ss << "while(" << iter_varname << ".has_next())\n{\n";
  m_output_ss << "BlockCluster "<<block_varname<<"{"<< iter_varname <<".next()->get_raw()};\n";

  m_types.insert(m_types.end(),q_ctype);
  ConsumeVisitor consume{m_output_ss,
                         block_varname,
                         m_types,
                         scan};

  scan->p_parent->accept(&consume);

  m_output_ss << "}\n\n";
}

void
ProduceVisitor::visit(const Join* join)
{
  m_output_ss << "std::unordered_map<int32_t,BlockCluster> hashtable;\n";
  ProduceVisitor produce{m_output_ss};
  join->p_left->accept(&produce);
  join->p_right->accept(&produce);

}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  std::string tag = tag_filter->m_tag;
  ProduceVisitor produce{m_output_ss};
  tag_filter->p_child->accept(&produce);
  m_types.insert(m_types.end(), produce.m_types.begin(), produce.m_types.end());
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  ProduceVisitor produce{m_output_ss};
  component_filter->p_child->accept(&produce);
  m_types.insert(m_types.end(), produce.m_types.begin(), produce.m_types.end());
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  ProduceVisitor produce{m_output_ss};
  predicate_filter->p_child->accept(&produce);
  m_types.insert(m_types.end(), produce.m_types.begin(), produce.m_types.end());
}

} /* furious */ 
