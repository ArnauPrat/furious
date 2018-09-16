

#include "consume_visitor.h"
#include "produce_visitor.h"

namespace furious 
{


ConsumeVisitor::ConsumeVisitor(std::stringstream& output_ss)  :
m_output_ss{output_ss}
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
}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss} ;
  tag_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss};
    component_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss};
  predicate_filter->p_parent->accept(&consume);

}

} /* furious */ 
