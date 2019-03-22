
#include "frontend/transforms.h"
#include "frontend/dep_graph.h"
#include "execution_plan.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious {

FccOperator::FccOperator(FccOperatorType type, 
                         const std::string& name,
                         FccContext* fcc_context) :
m_type(type),
m_name(name),
p_fcc_context(fcc_context),
p_parent(nullptr)
{
  static uint32_t id = 0;
  m_id = id;
  ++id;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Scan::Scan(const std::string& ref_name, FccContext* fcc_context) : 
  FccOperatorTmplt<Scan>(FccOperatorType::E_SCAN, "Scan", fcc_context) 
{
  FccColumn column;
  column.m_type = FccColumnType::E_REFERENCE;
  column.m_ref_name = ref_name;
  column.m_access_mode = FccAccessMode::E_READ;
  m_columns.append(column);

}

Scan::Scan(QualType component, FccAccessMode access_mode, FccContext* fcc_context) : 
  FccOperatorTmplt<Scan>(FccOperatorType::E_SCAN, "Scan", fcc_context) 
{
  FccColumn column;
  column.m_type = FccColumnType::E_COMPONENT;
  column.m_q_type = component;
  column.m_access_mode = access_mode;
  m_columns.append(column);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Join::Join(RefCountPtr<FccOperator> left, 
           RefCountPtr<FccOperator> right,
           FccContext* fcc_context) :
FccOperatorTmplt<Join>(FccOperatorType::E_JOIN, "Join", fcc_context), 
p_left(left),
p_right(right) 
{
  p_left.get()->p_parent = this;
  p_right.get()->p_parent = this;
  m_columns.append(p_left.get()->m_columns);
  m_split_point = m_columns.size();
  m_columns.append(p_right.get()->m_columns);
}

Join::~Join() 
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

PredicateFilter::PredicateFilter(RefCountPtr<FccOperator> child,
                                 const FunctionDecl* func_decl,
                                 FccContext* fcc_context) :
Filter<PredicateFilter>(child, "PredicateFilter",fcc_context),
p_func_decl(func_decl)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


TagFilter::TagFilter(RefCountPtr<FccOperator> child,
                     const std::string& tag,
                     FccFilterOpType op_type,
                     FccContext* fcc_context) :
Filter(child, "TagFilter",fcc_context),
m_tag(tag),
m_op_type(op_type)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

ComponentFilter::ComponentFilter(RefCountPtr<FccOperator> child,
                                 QualType component_type,
                                 FccFilterOpType op_type,
                                 FccContext* fcc_context) :
Filter<ComponentFilter>(child, "ComponentFilter", fcc_context),
m_filter_type(component_type),
m_op_type(op_type)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Foreach::Foreach(RefCountPtr<FccOperator> child,
                 const DynArray<const FccSystem*>& systems,
                 FccContext* fcc_context) :
  FccOperatorTmplt<Foreach>(FccOperatorType::E_FOREACH, "Foreach", fcc_context), 
  p_systems(systems), 
  p_child(child) 
{
  p_child.get()->p_parent = this;
  m_columns.append(child.get()->m_columns);
}

Foreach::~Foreach() 
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


Gather::Gather(RefCountPtr<FccOperator> ref_table,
               RefCountPtr<FccOperator> child,
               FccContext* fcc_context) :
FccOperatorTmplt<Gather>(FccOperatorType::E_GATHER, "Gather", fcc_context),
p_ref_table(ref_table),
p_child(child)
{
  p_ref_table.get()->p_parent=this;
  p_child.get()->p_parent = this;
  m_columns.append(child.get()->m_columns);
}

Gather::~Gather()
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

CascadingGather::CascadingGather(RefCountPtr<FccOperator> ref_table,
                                 RefCountPtr<FccOperator> child,
                                 FccContext* fcc_context) :
FccOperatorTmplt<CascadingGather>(FccOperatorType::E_CASCADING_GATHER, "CascadingGather", fcc_context),
p_ref_table(ref_table),
p_child(child)
{
  p_ref_table.get()->p_parent=this;
  p_child.get()->p_parent = this;
  m_columns.append(child.get()->m_columns);
}

CascadingGather::~CascadingGather()
{
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
