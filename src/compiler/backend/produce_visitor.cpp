

#include "codegen.h"
#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"

#include <time.h>
#include <stdlib.h>
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
  produce(p_context->p_fd,foreach->p_child);
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

  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", iter_varname.c_str(), table_varname.c_str());
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());
  fprintf(p_context->p_fd, "BlockCluster %s{%s.next().get_raw()};\n", block_varname.c_str(), iter_varname.c_str());

  consume(p_context->p_fd,
          scan->p_parent,
          block_varname,
          {q_ctype},
          scan);

  fprintf(p_context->p_fd,"}\n\n");
}

void
ProduceVisitor::visit(const Join* join)
{
  p_context->m_join_id = std::to_string((uint32_t)rand());
  std::string hashtable = "hashtable_" + p_context->m_join_id;
  fprintf(p_context->p_fd,"std::unordered_map<int32_t,BlockCluster> %s;\n", hashtable.c_str());
  produce(p_context->p_fd,join->p_left);
  produce(p_context->p_fd,join->p_right);
}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  produce(p_context->p_fd,tag_filter->p_child);
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  produce(p_context->p_fd,component_filter->p_child);
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  produce(p_context->p_fd,predicate_filter->p_child);
}

} /* furious */ 
