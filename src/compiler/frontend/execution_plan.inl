
namespace furious 
{
  
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename T>
FccOperatorTmplt<T>::FccOperatorTmplt(FccOperatorType type,
                                      const std::string& name,
                                      FccContext* fcc_context) : 
FccOperator(type, name, fcc_context)
{

}

template<typename T>
void
FccOperatorTmplt<T>::accept(FccExecPlanVisitor* visitor) const 
{
  visitor->visit((const T*)(this));
}

template<typename T>
Filter<T>::Filter(RefCountPtr<FccOperator> child, const std::string& name, FccContext* fcc_context) :
FccOperatorTmplt<T>(FccOperatorType::E_FILTER, name, fcc_context),
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
