
#include "exec_plan.h"

namespace furious 
{

  
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename T>
FccOperatorTmplt<T>::FccOperatorTmplt(FccOperatorType type,
                                      const char* name) : 
FccOperator(type, name)
{

}

template<typename T>
void
FccOperatorTmplt<T>::accept(FccSubPlanVisitor* visitor) const 
{
  visitor->visit((const T*)(this));
}

template<typename T>
Filter<T>::Filter(RefCountPtr<FccOperator> child, const char* name) :
FccOperatorTmplt<T>(FccOperatorType::E_FILTER, name),
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
