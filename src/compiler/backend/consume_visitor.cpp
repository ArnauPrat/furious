

#include "consume_visitor.h"
#include "produce_visitor.h"

namespace furious 
{

extern ProduceVisitor* produce;

ConsumeVisitor::ConsumeVisitor(std::ofstream& output_file)  :
m_output_file{output_file}
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
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
}

} /* furious */ 
