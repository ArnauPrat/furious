

#include "produce_visitor.h"
#include "consume_visitor.h"

namespace furious 
{

extern ConsumeVisitor* consume;
extern ProduceVisitor* produce;

ProduceVisitor::ProduceVisitor(std::ofstream& output_file) :
m_output_file{output_file}
{
}

void 
ProduceVisitor::visit(const Foreach* foreach)
{
}

void 
ProduceVisitor::visit(const Scan* scan)
{
}

void
ProduceVisitor::visit(const Join* join)
{
  join->p_left->accept(produce);
  join->p_right->accept(produce);
}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
}

} /* furious */ 
