
#include "operator.h"

namespace furious 
{

FccOperator::FccOperator(FccOperatorType type, 
                         const std::string& name) :
m_type(type),
m_name(name),
p_parent(nullptr)
{
  static uint32_t id = 0;
  m_id = id;
  ++id;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Scan::Scan(const std::string& ref_name) : 
FccOperatorTmplt<Scan>(FccOperatorType::E_SCAN, "Scan") 
{
  FccColumn column;
  column.m_type = FccColumnType::E_REFERENCE;
  column.m_ref_name = ref_name;
  column.m_access_mode = FccAccessMode::E_READ;
  m_columns.append(column);
}

Scan::Scan(QualType component, 
           FccAccessMode access_mode) : 
FccOperatorTmplt<Scan>(FccOperatorType::E_SCAN, "Scan") 
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
           RefCountPtr<FccOperator> right) :
FccOperatorTmplt<Join>(FccOperatorType::E_JOIN, "Join"), 
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

LeftFilterJoin::LeftFilterJoin(RefCountPtr<FccOperator> left, 
                               RefCountPtr<FccOperator> right) :
FccOperatorTmplt<LeftFilterJoin>(FccOperatorType::E_LEFT_FILTER_JOIN, "LeftFilterJoin"), 
p_left(left),
p_right(right) 
{
  p_left.get()->p_parent = this;
  p_right.get()->p_parent = this;
  m_columns.append(p_left.get()->m_columns);
}

LeftFilterJoin::~LeftFilterJoin() 
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

CrossJoin::CrossJoin(RefCountPtr<FccOperator> left, 
                     RefCountPtr<FccOperator> right) :
FccOperatorTmplt<CrossJoin>(FccOperatorType::E_CROSS_JOIN, "CrossJoin"), 
p_left(left),
p_right(right) 
{
  p_left.get()->p_parent = this;
  p_right.get()->p_parent = this;
  m_columns.append(p_left.get()->m_columns);
  m_split_point = m_columns.size();
  m_columns.append(p_right.get()->m_columns);
}

CrossJoin::~CrossJoin() 
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


Fetch::Fetch(QualType global_type,
             FccAccessMode access_mode) : 
FccOperatorTmplt<Fetch>(FccOperatorType::E_FETCH, "Fetch"),
m_global_type(global_type)
{
  FccColumn column;
  column.m_access_mode = access_mode;
  column.m_q_type = global_type;
  column.m_type = FccColumnType::E_GLOBAL;
  m_columns.append(column);
}

Fetch::~Fetch()
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

PredicateFilter::PredicateFilter(RefCountPtr<FccOperator> child,
                                 const FunctionDecl* func_decl) :
Filter<PredicateFilter>(child, "PredicateFilter"),
p_func_decl(func_decl)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


TagFilter::TagFilter(RefCountPtr<FccOperator> child,
                     const std::string& tag,
                     FccFilterOpType op_type,
                     bool on_column) :
Filter(child, "TagFilter"),
m_tag(tag),
m_on_column(on_column),
m_op_type(op_type)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

ComponentFilter::ComponentFilter(RefCountPtr<FccOperator> child,
                                 QualType component_type,
                                 FccFilterOpType op_type,
                                 bool on_column) :
Filter<ComponentFilter>(child, "ComponentFilter"),
m_filter_type(component_type),
m_on_column(on_column),
m_op_type(op_type)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Foreach::Foreach(RefCountPtr<FccOperator> child,
                 const DynArray<const FccSystem*>& systems) :
FccOperatorTmplt<Foreach>(FccOperatorType::E_FOREACH, "Foreach"), 
p_systems(systems), 
p_child(child) 
{
  if(child.get() != nullptr)
  {
    p_child.get()->p_parent = this;
    m_columns.append(child.get()->m_columns);
  }
}

Foreach::~Foreach() 
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


Gather::Gather(RefCountPtr<FccOperator> ref_table,
               RefCountPtr<FccOperator> child) :
FccOperatorTmplt<Gather>(FccOperatorType::E_GATHER, "Gather"),
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
                                 RefCountPtr<FccOperator> child) :
FccOperatorTmplt<CascadingGather>(FccOperatorType::E_CASCADING_GATHER, "CascadingGather"),
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

}
