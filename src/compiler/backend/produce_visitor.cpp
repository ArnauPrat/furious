

#include "codegen.h"
#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"

#include <stdlib.h>
#include <clang/AST/PrettyPrinter.h>
#include <string>
#include <algorithm>

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
  produce(p_context->p_fd,foreach->p_child.get());
}

void 
ProduceVisitor::visit(const Scan* scan)
{
  static uint32_t id = 0;
  const FccColumn* column = &scan->m_columns[0];
  std::string ctype = get_type_name(column->m_q_type);
  std::string q_ctype = get_qualified_type_name(column->m_q_type);
  std::string base_name = ctype;

  std::transform(base_name.begin(), 
                 base_name.end(), 
                 base_name.begin(), ::tolower);
  std::replace(base_name.begin(),
                    base_name.end(),
                    ':',
                    '_');

  std::string table_varname = base_name+"_table";
  std::string iter_varname = base_name+"_iter_" + std::to_string(id);
  id++;
  std::string block_varname = base_name+"_block"; 

  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", iter_varname.c_str(), table_varname.c_str());
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());
  fprintf(p_context->p_fd, "BlockCluster %s(%s.next().get_raw());\n", block_varname.c_str(), iter_varname.c_str());

  consume(p_context->p_fd,
          scan->p_parent,
          "(&"+block_varname+")",
          scan);

  fprintf(p_context->p_fd,"}\n\n");
}

void
ProduceVisitor::visit(const Join* join)
{
  std::string hashtable = "hashtable_" + std::to_string(join->m_id);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable.c_str());
  produce(p_context->p_fd,join->p_left.get());
  produce(p_context->p_fd,join->p_right.get());
}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  produce(p_context->p_fd,tag_filter->p_child.get());
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  produce(p_context->p_fd,component_filter->p_child.get());
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  produce(p_context->p_fd,predicate_filter->p_child.get());
}

void
ProduceVisitor::visit(const Gather* gather)
{
  produce(p_context->p_fd, gather->p_child.get());
}

} /* furious */ 
