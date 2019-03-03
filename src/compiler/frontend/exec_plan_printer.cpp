

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
    SourceLocation start = info->m_ctor_params[0]->getLocStart();
    SourceLocation end = info->m_ctor_params[0]->getLocEnd();
    str_builder.append("%s", get_code(sm,start,end).c_str());

    for (int32_t i = 1; i < (int32_t)info->m_ctor_params.size(); ++i) 
    {
      SourceLocation start = info->m_ctor_params[i]->getLocStart();
      SourceLocation end = info->m_ctor_params[i]->getLocEnd();
      str_builder.append(", %s", get_code(sm, start, end).c_str()); 
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
  str_builder.append("foreach (%lu) - \"%s\"",
                     foreach, 
                     to_string(info).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  foreach->p_child->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const Scan* scan) 
{
  StringBuilder str_builder;
  str_builder.append("scan (%lu) - \"%s\"",
                     scan, 
                     scan->m_component->getAsCXXRecordDecl()->getNameAsString().c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  decr_level();
}

void
ExecPlanPrinter::visit(const Join* join) 
{
  StringBuilder str_builder;
  str_builder.append("join(%lu)", join);
  print(str_builder.p_buffer);
  incr_level(true);
  join->p_left->accept(this);
  join->p_right->accept(this);
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

  str_builder.append("tag_filter (%lu) - %s - \"%s\"", 
                     tag_filter, 
                     type, 
                     tag_filter->m_tag.c_str()); 
  print(str_builder.p_buffer);
  incr_level(false);
  tag_filter->p_child->accept(this);
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
  str_builder.append("component_filter (%lu) - %s - \"%s\"", 
                     component_filter, 
                     type, 
                     component_filter->m_component_type->getAsCXXRecordDecl()->getNameAsString().c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  component_filter->p_child->accept(this);
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
  str_builder.append("predicate_filter (%lu) - %s", 
                     predicate_filter, 
                     to_string(predicate_filter->p_func_decl).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  predicate_filter->p_child->accept(this);
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
