

#include "exec_plan_printer.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious
{

ExecPlanPrinter::ExecPlanPrinter(bool add_comments)
{
  if(add_comments)
  {
    m_offsets.push_back('/');
    m_offsets.push_back('/');
  }
}


void
ExecPlanPrinter::traverse(const FccExecPlan* plan) 
{
  for(int32_t i = 0; i < (int32_t)plan->m_roots.size(); ++i)
  {
    plan->m_roots[i]->accept(this);
  }
}

std::string
to_string(const FccSystemInfo* info)
{
  std::stringstream ss;
  ss << info->m_system_type->getAsCXXRecordDecl()->getNameAsString() << "(";
  if(info->m_ctor_params.size() > 0)
  {
    const ASTContext& context = info->m_system_type->getAsCXXRecordDecl()->getASTContext();
    const SourceManager& sm = context.getSourceManager();
    SourceLocation start = info->m_ctor_params[0]->getLocStart();
    SourceLocation end = info->m_ctor_params[0]->getLocEnd();
    ss << get_code(sm, start, end); 

    for (int32_t i = 1; i < (int32_t)info->m_ctor_params.size(); ++i) 
    {
      SourceLocation start = info->m_ctor_params[i]->getLocStart();
      SourceLocation end = info->m_ctor_params[i]->getLocEnd();
      ss << "," << get_code(sm, start, end); 
    }

  }
  ss << ")";
  return ss.str();
}

void 
ExecPlanPrinter::visit(const Foreach* foreach) 
{
  std::stringstream ss;
  ss << "foreach ("<< foreach <<  ") - " 
     << "\"" << to_string(&foreach->m_systems[0]) << "\"";
  print(ss.str());
  incr_level(false);
  foreach->p_child->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const Scan* scan) 
{
  std::stringstream ss;
  ss << "scan (" << scan << ") - " 
     << "\"" << scan->m_component->getAsCXXRecordDecl()->getNameAsString() << "\"";
  print(ss.str());
  incr_level(false);
  decr_level();
}

void
ExecPlanPrinter::visit(const Join* join) 
{
  std::stringstream ss;
  ss << "join (" << join << ")";
  print(ss.str());
  incr_level(true);
  join->p_left->accept(this);
  join->p_right->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const TagFilter* tag_filter) 
{
  std::stringstream ss;
  ss << "tag_filter (" << tag_filter << ") - ";
  if(tag_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    ss << "has ";
  }
  else
  {
    ss << "has not ";
  }
  ss << "\"" << tag_filter->m_tag << "\"";
  print(ss.str());
  incr_level(false);
  tag_filter->p_child->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const ComponentFilter* component_filter) 
{
  std::stringstream ss;
  ss << "component_filter (" << component_filter << ") - ";
  if(component_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    ss << "has ";
  }
  else
  {
    ss << "has not ";
  }
  ss << "\"" << component_filter->m_component_type->getAsCXXRecordDecl()->getNameAsString() << "\"";
  print(ss.str());
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
  std::stringstream ss;
  ss << "predicate_filter (" << predicate_filter << ") - " << to_string(predicate_filter->p_func_decl);
  print(ss.str());
  incr_level(false);
  predicate_filter->p_child->accept(this);
  decr_level();
}

void
ExecPlanPrinter::incr_level(bool sibling)
{
  if(sibling)
  {
    m_offsets.push_back(' ');
    m_offsets.push_back('|');
  } 
  else 
  {
    m_offsets.push_back(' ');
    m_offsets.push_back(' ');
  }
}

void
ExecPlanPrinter::decr_level()
{
  m_offsets.pop_back();
  m_offsets.pop_back();
}

void
ExecPlanPrinter::print(const std::string& str)
{
  for(int32_t i = 0;
      i < (int32_t)m_offsets.size();
      ++i) 
  {
    m_string_builder << m_offsets[i];
  }
  m_string_builder << "-" << str << "\n";
}

} /* furious
*/ 
