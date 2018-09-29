

#include "codegen.h"
#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"

#include <clang/AST/PrettyPrinter.h>
#include <string>

using namespace clang;

namespace furious 
{

ProduceVisitor::ProduceVisitor(CodeGenContext* context) :
p_context{context}
{
}

void 
ProduceVisitor::visit(const Foreach* foreach)
{
  produce(p_context->m_output_ss,foreach->p_child);
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

  p_context->m_output_ss << "auto " + iter_varname << " = " << table_varname << ".iterator();\n";
  p_context->m_output_ss << "while(" << iter_varname << ".has_next())\n{\n";
  p_context->m_output_ss << "BlockCluster "<<block_varname<<"{"<< iter_varname <<".next()->get_raw()};\n";

  consume(p_context->m_output_ss,
          scan->p_parent,
          block_varname,
          {q_ctype},
          scan);

  p_context->m_output_ss << "}\n\n";
}

void
ProduceVisitor::visit(const Join* join)
{

  p_context->m_hashtable_name = "hash_X";
  p_context->m_output_ss << "std::unordered_map<int32_t,BlockCluster> "<<p_context->m_hashtable_name<<";\n";
  produce(p_context->m_output_ss,join->p_left);
  produce(p_context->m_output_ss,join->p_right);
}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  produce(p_context->m_output_ss,tag_filter->p_child);
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  produce(p_context->m_output_ss,component_filter->p_child);
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  produce(p_context->m_output_ss,predicate_filter->p_child);
}

} /* furious */ 
