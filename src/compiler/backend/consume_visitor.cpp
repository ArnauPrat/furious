

#include "consume_visitor.h"
#include "produce_visitor.h"

namespace furious 
{


ConsumeVisitor::ConsumeVisitor(std::stringstream& output_ss,
                               const std::string& source,
                               const FccOperator* caller)  :
m_source{source},
m_output_ss{output_ss},
p_caller{caller}
{
}

void 
ConsumeVisitor::visit(const Foreach* foreach)
{
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{
  if(p_caller == join->p_left) 
  {
    m_output_ss << "hashtable[" << m_source << ".m_start] = " << m_source << ";\n"; 
  } else 
  {
    m_output_ss << "auto it = hashtable.find(" << m_source << ".m_start);\n";
    m_output_ss << "if(it != hashtable.end()) {\n";
    m_output_ss << "auto& cluster = *it;\n";
    m_output_ss << "append(&cluster,&" << m_source<< ");\n";
    ConsumeVisitor consume{m_output_ss, "cluster", join};
    join->p_parent->accept(&consume);
    m_output_ss << "}\n";
  }
}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss, m_source, tag_filter} ;
  tag_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss, m_source, component_filter};
    component_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss, m_source, predicate_filter};
  predicate_filter->p_parent->accept(&consume);

}

} /* furious */ 
