
#include "operator.h"
#include "string.h"

namespace furious 
{

fcc_operator_t::fcc_operator_t(fcc_operator_type_t type, 
                         const char* name) :
m_type(type),
p_parent(nullptr)
{
  strncpy(m_name, name, MAX_OPERATOR_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(name), MAX_OPERATOR_NAME);
  static uint32_t id = 0;
  m_id = id;
  ++id;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Scan::Scan(const char* ref_name) : 
fcc_operator_tmplt_t<Scan>(fcc_operator_type_t::E_SCAN, "Scan") 
{
  fcc_column_t column;
  column.m_type = fcc_column_type_t::E_REFERENCE;
  strncpy(column.m_ref_name,ref_name, MAX_REF_NAME);
  column.m_access_mode = fcc_access_mode_t::E_READ;
  m_columns.append(column);
}

Scan::Scan(fcc_type_t component, 
           fcc_access_mode_t access_mode) : 
fcc_operator_tmplt_t<Scan>(fcc_operator_type_t::E_SCAN, "Scan") 
{
  fcc_column_t column;
  column.m_type = fcc_column_type_t::E_COMPONENT;
  column.m_component_type = component;
  column.m_access_mode = access_mode;
  m_columns.append(column);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Join::Join(RefCountPtr<fcc_operator_t> left, 
           RefCountPtr<fcc_operator_t> right) :
fcc_operator_tmplt_t<Join>(fcc_operator_type_t::E_JOIN, "Join"), 
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

LeftFilterJoin::LeftFilterJoin(RefCountPtr<fcc_operator_t> left, 
                               RefCountPtr<fcc_operator_t> right) :
fcc_operator_tmplt_t<LeftFilterJoin>(fcc_operator_type_t::E_LEFT_FILTER_JOIN, "LeftFilterJoin"), 
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

CrossJoin::CrossJoin(RefCountPtr<fcc_operator_t> left, 
                     RefCountPtr<fcc_operator_t> right) :
fcc_operator_tmplt_t<CrossJoin>(fcc_operator_type_t::E_CROSS_JOIN, "CrossJoin"), 
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


Fetch::Fetch(fcc_type_t global_type,
             fcc_access_mode_t access_mode) : 
fcc_operator_tmplt_t<Fetch>(fcc_operator_type_t::E_FETCH, "Fetch"),
m_global_type(global_type)
{
  fcc_column_t column;
  column.m_access_mode = access_mode;
  column.m_component_type = global_type;
  column.m_type = fcc_column_type_t::E_GLOBAL;
  m_columns.append(column);
}

Fetch::~Fetch()
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

PredicateFilter::PredicateFilter(RefCountPtr<fcc_operator_t> child,
                                 fcc_decl_t func_decl) :
Filter<PredicateFilter>(child, fcc_operator_type_t::E_PREDICATE_FILTER, "PredicateFilter"),
m_func_decl(func_decl)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


TagFilter::TagFilter(RefCountPtr<fcc_operator_t> child,
                     const char* tag,
                     FccFilterOpType op_type,
                     bool on_column) :
Filter(child, fcc_operator_type_t::E_TAG_FILTER, "TagFilter"),
m_on_column(on_column),
m_op_type(op_type)
{
  strncpy(m_tag, tag, MAX_TAG_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(tag), MAX_TAG_NAME);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

ComponentFilter::ComponentFilter(RefCountPtr<fcc_operator_t> child,
                                 fcc_type_t component_type,
                                 FccFilterOpType op_type,
                                 bool on_column) :
Filter<ComponentFilter>(child, 
                        fcc_operator_type_t::E_COMPONENT_FILTER,
                        "ComponentFilter"),
m_filter_type(component_type),
m_on_column(on_column),
m_op_type(op_type)
{
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

Foreach::Foreach(RefCountPtr<fcc_operator_t> child,
                 const DynArray<const fcc_system_t*>& systems) :
fcc_operator_tmplt_t<Foreach>(fcc_operator_type_t::E_FOREACH, "Foreach"), 
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


Gather::Gather(RefCountPtr<fcc_operator_t> ref_table,
               RefCountPtr<fcc_operator_t> child) :
fcc_operator_tmplt_t<Gather>(fcc_operator_type_t::E_GATHER, "Gather"),
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

CascadingGather::CascadingGather(RefCountPtr<fcc_operator_t> ref_table,
                                 RefCountPtr<fcc_operator_t> child) :
fcc_operator_tmplt_t<CascadingGather>(fcc_operator_type_t::E_CASCADING_GATHER, "CascadingGather"),
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
