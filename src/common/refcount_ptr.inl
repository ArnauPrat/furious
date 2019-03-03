

namespace furious
{

inline RefData::RefData() :
m_num_refs(0)
{
}

template <typename T>
RefCountPtr<T>::RefCountPtr(T* ptr) : 
p_ptr(ptr)
{
  p_ref_data = new RefData();
  lock();
}

template <typename T>
RefCountPtr<T>::RefCountPtr(const RefCountPtr& other) : 
p_ptr(other.p_ptr),
p_ref_data(other.p_ref_data)
{
  lock();
}
  
template <typename T>
RefCountPtr<T>::~RefCountPtr()
{
  release();
  if(p_ref_data->m_num_refs == 0)
  {
    delete p_ptr;
    delete p_ref_data;
  }
}

template <typename T>
T*
RefCountPtr<T>::get() const
{
  return p_ptr;
}

template <typename T>
void
RefCountPtr<T>::lock()
{
  ++p_ref_data->m_num_refs;
}

template <typename T>
void
RefCountPtr<T>::release()
{
  --p_ref_data->m_num_refs;
}

template <typename T>
RefCountPtr<T>&
RefCountPtr<T>::operator=(const RefCountPtr<T>& other)
{
  release();
  if(p_ref_data->m_num_refs == 0)
  {
    delete p_ptr;
    delete p_ref_data;
  }
  p_ptr = other.p_ptr;
  p_ref_data = other.p_ref_data;
  lock();
}


} /* furious */ 
