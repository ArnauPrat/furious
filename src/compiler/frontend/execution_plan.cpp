
#include "frontend/transforms.h"
#include "frontend/dep_graph.h"
#include "execution_plan.h"
#include "fcc_context.h"

namespace furious {

FccOperator::FccOperator(FccOperatorType type) :
m_type(type),
p_parent(nullptr)
{
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Scan::Scan(QualType component) : 
  FccOperatorTmplt<Scan>(FccOperatorType::E_SCAN), 
  m_component(component) 
{

}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Join::Join(FccOperator* left, 
           FccOperator* right) :
FccOperatorTmplt<Join>(FccOperatorType::E_JOIN), 
p_left(left),
p_right(right) 
{
  p_left->p_parent = this;
  p_right->p_parent = this;
}

Join::~Join() {
  if(p_left != nullptr) {
    delete p_left;
  }

  if(p_right != nullptr) {
    delete p_right;
  }
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

PredicateFilter::PredicateFilter(FccOperator* child,
                                 const FunctionDecl* func_decl) :
Filter<PredicateFilter>{child},
p_func_decl{func_decl}
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


TagFilter::TagFilter(FccOperator* child,
                     const std::string& tag,
                     FccFilterOpType op_type) :
Filter{child},
m_tag{tag},
m_op_type{op_type}
{

}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

ComponentFilter::ComponentFilter(FccOperator* child,
                                 QualType component_type,
                                 FccFilterOpType op_type) :
Filter<ComponentFilter>{child},
m_component_type{component_type},
m_op_type{op_type}
{

}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Foreach::Foreach(FccOperator* child,
                 const DynArray<const FccSystem*>& systems) :
  FccOperatorTmplt<Foreach>(FccOperatorType::E_FOREACH), 
  p_systems(systems), 
  p_child(child) 
{
  p_child->p_parent = this;
}

Foreach::~Foreach() {
  if(p_child != nullptr) {
    delete p_child;
  }
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


FccExecPlan::FccExecPlan(FccContext* context) :
p_context(context)
{

}

FccExecPlan::~FccExecPlan()
{
  for(uint32_t i = 0; i < p_roots.size(); ++i)
  {
    destroy_subplan(p_roots[i]);
  }
}

void
FccExecPlan::insert_root(ASTContext* ast_context, 
                         FccOperator* root)
{
  p_roots.append(root); 
  p_asts.append(ast_context);
}

bool
FccExecPlan::bootstrap()
{
  DependencyGraph dep_graph;
  uint32_t size = p_context->p_matches.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    dep_graph.insert(p_context->p_matches[i]);
  }

  if(!dep_graph.is_acyclic()) 
  {
    return false;
  }

  DynArray<const FccMatch*> seq = dep_graph.get_valid_exec_sequence();
  for(uint32_t i = 0; i < seq.size(); ++i)
  {
    const FccMatch* info = seq[i];
    FccOperator* next_root = create_subplan(info);
    insert_root(info->p_ast_context, next_root);
  }
  return true;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
FccExecPlanVisitor::traverse(const FccExecPlan* plan)
{
  for(uint32_t i = 0; i < plan->p_roots.size(); ++i)
  {
    plan->p_roots[i]->accept(this);
  }
}


} /* furious */ 
