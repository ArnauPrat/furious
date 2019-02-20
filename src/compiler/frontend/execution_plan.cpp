
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
                 const DynArray<const FccSystemInfo*>& systems) :
  FccOperatorTmplt<Foreach>(FccOperatorType::E_FOREACH), 
  m_systems(systems), 
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
p_context(context),
m_num_asts(0),
m_max_asts(0),
m_num_roots(0),
m_max_roots(0),
m_asts(NULL),
m_roots(NULL)
{

}

FccExecPlan::~FccExecPlan()
{
  if(m_asts != NULL)
  {
    free(m_asts);
  }

  for(uint32_t i = 0;  i < m_num_roots; ++i)
  {
    if(m_roots[i] != NULL)
    {
      delete m_roots[i];
      m_roots[i] = NULL;
    }
  }

  if(m_roots != NULL)
  {
    free(m_roots);
  }
}

#define FCC_EXEC_PLAN_GROWING_FACTOR 16 

void
FccExecPlan::insert_root(ASTContext* ast_context, const FccOperator* root)
{
  if(m_num_roots == m_max_roots)
  {
    m_max_roots += FCC_EXEC_PLAN_GROWING_FACTOR;
    m_roots = (const FccOperator**)realloc(m_roots, m_max_roots*sizeof(const FccOperator**));

    m_max_asts += FCC_EXEC_PLAN_GROWING_FACTOR;
    m_asts = (ASTContext**)realloc(m_asts, m_max_asts*sizeof(ASTContext*));
  }

  m_roots[m_num_roots] = root; 
  m_num_roots++;
  m_asts[m_num_asts] = ast_context;
  m_num_asts++;

}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
FccExecPlanVisitor::traverse(const FccExecPlan* plan)
{
  for(uint32_t i = 0; i < plan->m_num_roots; ++i)
  {
    plan->m_roots[i]->accept(this);
  }
}

} /* furious */ 
