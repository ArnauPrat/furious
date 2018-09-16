

#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"

namespace furious 
{

ProduceVisitor::ProduceVisitor(std::stringstream& output_ss) :
m_output_ss{output_ss}
{
}

void 
ProduceVisitor::visit(const Foreach* foreach)
{
  ProduceVisitor produce{m_output_ss};
  foreach->p_child->accept(&produce);
}

void 
ProduceVisitor::visit(const Scan* scan)
{
  QualType component = scan->m_component;
  const Decl* decl = nullptr;
  if(component->isAnyPointerType()) 
  {
    decl = component->getPointeeCXXRecordDecl();
  } else 
  {
    decl = component->getAsCXXRecordDecl();
  }
  ASTContext& context = decl->getASTContext();
  std::vector<Dependency> dependencies = get_dependencies(&context, decl);
  for(auto& dep : dependencies)
  {
    llvm::errs() << dep.m_include_file << "\n";
  }

}

void
ProduceVisitor::visit(const Join* join)
{
  ProduceVisitor produce1{m_output_ss};
  join->p_left->accept(&produce1);

  ProduceVisitor produce2{m_output_ss};
  join->p_right->accept(&produce2);
}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  ProduceVisitor produce{m_output_ss};
  tag_filter->p_child->accept(&produce);
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  ProduceVisitor produce{m_output_ss};
  component_filter->p_child->accept(&produce);
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  ProduceVisitor produce{m_output_ss};
  predicate_filter->p_child->accept(&produce);
}

} /* furious */ 
