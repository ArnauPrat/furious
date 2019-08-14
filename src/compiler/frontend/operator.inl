
#include "exec_plan.h"

namespace furious 
{

  
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename T>
fcc_operator_tmplt_t<T>::fcc_operator_tmplt_t(fcc_operator_type_t type,
                                      const char* name) : 
fcc_operator_t(type, name)
{

}

template<typename T>
void
fcc_operator_tmplt_t<T>::accept(FccSubPlanVisitor* visitor) const 
{
  visitor->visit((const T*)(this));
}

template<typename T>
Filter<T>::Filter(RefCountPtr<fcc_operator_t> child, 
                  fcc_operator_type_t type,
                  const char* name) :
fcc_operator_tmplt_t<T>(type, name),
p_child(child) 
{
  p_child.get()->p_parent = this;
  this->m_columns.append(p_child.get()->m_columns);
}

template<typename T>
Filter<T>::~Filter() 
{
}

} /* furious */ 
