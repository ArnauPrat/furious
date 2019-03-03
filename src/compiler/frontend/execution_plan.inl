
namespace furious 
{
  
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

template<typename T>
FccOperatorTmplt<T>::FccOperatorTmplt(FccOperatorType type) : 
FccOperator{type}
{

}

template<typename T>
void
FccOperatorTmplt<T>::accept(FccExecPlanVisitor* visitor) const 
{
  visitor->visit((const T*)(this));
}

template<typename T>
Filter<T>::Filter(RefCountPtr<FccOperator> child) :
FccOperatorTmplt<T>(FccOperatorType::E_FILTER),
p_child(child) 
{
  p_child.get()->p_parent = this;
}

template<typename T>
Filter<T>::~Filter() 
{
}

} /* furious */ 
