

#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"

#include <clang/AST/PrettyPrinter.h>
#include <string>

using namespace clang;

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


  const std::string component_type_name = get_type_name(scan->m_component);

  std::string table_varname = gen_declare_tableview(m_output_ss, 
                                                    component_type_name);


  std::string iter_varname = gen_declare_biterator(m_output_ss, 
                                                   table_varname);

  std::string element_varname = gen_begin_biterate(m_output_ss, 
                                                   iter_varname);

  ConsumeVisitor consume{m_output_ss,
                         element_varname,
                         scan};

  scan->p_parent->accept(&consume);

  gen_end_biterate(m_output_ss);


}

void
ProduceVisitor::visit(const Join* join)
{
  std::string hashtable = gen_declare_bhashtable(m_output_ss);
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
