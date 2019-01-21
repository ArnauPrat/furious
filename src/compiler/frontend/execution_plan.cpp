
#include "execution_plan.h"
#include "fcc_context.h"

namespace furious {

FccOperator::FccOperator(FccOperatorType type) :
m_type(type),
p_parent{nullptr}
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
                 const std::vector<FccSystemInfo>& systems) :
  FccOperatorTmplt<Foreach>{FccOperatorType::E_FOREACH}, 
  m_systems{systems}, 
  p_child{child} 
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
p_context{context}
{

}

FccExecPlan::~FccExecPlan()
{
  for(auto root : m_roots)
  {
    delete root;
  }
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
FccExecPlanVisitor::traverse(const FccExecPlan* plan)
{
  for(const FccOperator* root : plan->m_roots)
  {
    root->accept(this);
  }
}

} /* furious */ 
