

#include "exec_plan_printer.h"

namespace furious
{

ExecPlanPrinter::ExecPlanPrinter()
{
  m_offsets.push_back('-');
  m_offsets.push_back('|');
}

void
ExecPlanPrinter::traverse(const FccExecPlan* plan) 
{
  for(const FccOperator* root : plan->m_roots)
  {
    root->accept(this);
  }
}

void 
ExecPlanPrinter::visit(const Foreach* foreach) 
{
  print("foreach");
  incr_level(false);
  foreach->p_child->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const Scan* scan) 
{
  print("scan");
  incr_level(false);
  decr_level();
}

void
ExecPlanPrinter::visit(const Join* join) 
{
  print("join");
  incr_level(true);
  join->p_left->accept(this);
  join->p_right->accept(this);
  decr_level();
}

void 
ExecPlanPrinter::visit(const TagFilter* tag_filter) 
{
  print("tag_filter");
  incr_level(false);
  tag_filter->p_child->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const ComponentFilter* component_filter) 
{
  print("component_filter");
  incr_level(false);
  component_filter->p_child->accept(this);
  decr_level();
}

void
ExecPlanPrinter::visit(const PredicateFilter* predicate_filter) 
{
  print("predicate_filter");
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
  for(int32_t i = m_offsets.size()-1;
      i >= 0;
      --i) 
  {
    m_string_builder << m_offsets[i];
  }
  m_string_builder << str << "\n";
}

} /* furious
*/ 
