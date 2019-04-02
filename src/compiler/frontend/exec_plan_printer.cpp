

#include "exec_plan_printer.h"
#include "fcc_context.h"
#include "clang_tools.h"
#include "../common/string_builder.h"

namespace furious
{

ExecPlanPrinter::ExecPlanPrinter(bool add_comments)
{
  if(add_comments)
  {
    m_offsets.append('/');
    m_offsets.append('/');
  }
}


void
ExecPlanPrinter::traverse(const FccExecPlan* plan) 
{
  for(uint32_t i = 0; i < plan->p_roots.size(); ++i)
  {
    plan->p_roots[i]->accept(this);
  }
}

std::string
to_string(const FccSystem* info)
{
  StringBuilder str_builder;
  str_builder.append("%s (", info->m_system_type->getAsCXXRecordDecl()->getNameAsString().c_str());
  if(info->m_ctor_params.size() > 0)
  {
    const ASTContext& context = info->m_system_type->getAsCXXRecordDecl()->getASTContext();
    const SourceManager& sm = context.getSourceManager();
    str_builder.append("%s", 
                       get_code(sm,info->m_ctor_params[0]->getSourceRange()).c_str());

    for (int32_t i = 1; i < (int32_t)info->m_ctor_params.size(); ++i) 
    {
      str_builder.append(", %s", 
                         get_code(sm, info->m_ctor_params[i]->getSourceRange()).c_str()); 
    }

  }
  str_builder.append(")");
  return str_builder.p_buffer;
}

void 
ExecPlanPrinter::visit(const Foreach* foreach) 
{
  StringBuilder str_builder;
  const FccSystem* info = foreach->p_systems[0];
  str_builder.append("foreach (%u) - \"%s\"",
                     foreach->m_id, 
                     to_string(info).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  foreach->p_child.get()->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const Scan* scan) 
{
  StringBuilder str_builder;
  const FccColumn* column = &scan->m_columns[0];
  if(column->m_type == FccColumnType::E_COMPONENT)
  {
    str_builder.append("scan (%u) - \"%s\"",
                       scan->m_id, 
                       get_type_name(column->m_q_type).c_str());

  }
  else
  {
    str_builder.append("scan (%u) - #REFERENCE \"%s\"",
                       scan->m_id, 
                       column->m_ref_name.c_str());
  }

  print(str_builder.p_buffer);
  incr_level(false);
  decr_level();
}

void
ExecPlanPrinter::visit(const Join* join) 
{
  StringBuilder str_builder;
  str_builder.append("join(%u)", join->m_id);
  print(str_builder.p_buffer);
  incr_level(true);
  join->p_left.get()->accept(this);
  join->p_right.get()->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const LeftFilterJoin* left_filter_join) 
{
  StringBuilder str_builder;
  str_builder.append("left_filter_join(%u)", left_filter_join->m_id);
  print(str_builder.p_buffer);
  incr_level(true);
  left_filter_join->p_left.get()->accept(this);
  left_filter_join->p_right.get()->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const TagFilter* tag_filter) 
{
  StringBuilder str_builder;
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(tag_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    type = has_type;
  }
  else
  {
    type = has_not_type;
  }

  char* column_type;
  char on_key[] = "on_key";
  char on_ref_column[] = "on_ref_column";
  if(tag_filter->m_on_column)
  {
    column_type = on_ref_column;
  }
  else
  {
    column_type = on_key;
  }

  str_builder.append("tag_filter (%u) - %s - \"%s\" - %s", 
                     tag_filter->m_id, 
                     type, 
                     tag_filter->m_tag.c_str(),
                     column_type); 
  print(str_builder.p_buffer);
  incr_level(false);
  tag_filter->p_child.get()->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const ComponentFilter* component_filter) 
{
  StringBuilder str_builder;
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(component_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    type = has_type;
  }
  else
  {
    type = has_not_type;
  }
  str_builder.append("component_filter (%u) - %s - \"%s\"", 
                     component_filter->m_id, 
                     type, 
                     get_type_name(component_filter->m_filter_type).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  component_filter->p_child.get()->accept(this);
  decr_level();
}

std::string
to_string(const FunctionDecl* func_decl)
{
  return func_decl->getNameAsString();
}

void
ExecPlanPrinter::visit(const PredicateFilter* predicate_filter) 
{
  StringBuilder str_builder;
  str_builder.append("predicate_filter (%u) - %s", 
                     predicate_filter->m_id, 
                     to_string(predicate_filter->p_func_decl).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  predicate_filter->p_child.get()->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const Gather* gather) 
{
  StringBuilder str_builder;
  FccColumn* ref_column = &gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != FccColumnType::E_REFERENCE)
  {
    //TODO: handle error
  }
  str_builder.append("gather (%u)", 
                     gather->m_id);
  print(str_builder.p_buffer);
  incr_level(false);
  gather->p_ref_table.get()->accept(this);
  gather->p_child.get()->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const CascadingGather* casc_gather) 
{
  StringBuilder str_builder;
  FccColumn* ref_column = &casc_gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != FccColumnType::E_REFERENCE)
  {
    //TODO: handle error
  }
  str_builder.append("cascading_gather (%u)", 
                     casc_gather->m_id);
  print(str_builder.p_buffer);
  incr_level(false);
  casc_gather->p_ref_table.get()->accept(this);
  casc_gather->p_child.get()->accept(this);
  decr_level();
}

void
ExecPlanPrinter::incr_level(bool sibling)
{
  if(sibling)
  {
    m_offsets.append(' ');
    m_offsets.append('|');
  } 
  else 
  {
    m_offsets.append(' ');
    m_offsets.append(' ');
  }
}

void
ExecPlanPrinter::decr_level()
{
  m_offsets.pop();
  m_offsets.pop();
}

void
ExecPlanPrinter::print(const std::string& str)
{
  for(int32_t i = 0;
      i < (int32_t)m_offsets.size();
      ++i) 
  {
    m_string_builder.append("%c", m_offsets[i]);
  }
  m_string_builder.append("- %s\n", str.c_str());
}

} /* furious
*/ 
