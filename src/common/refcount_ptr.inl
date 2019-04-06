

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
  if(p_ref_data != nullptr && 
     p_ref_data->m_num_refs == 0)
  {
    delete p_ref_data;
    p_ref_data = nullptr;
    if(p_ptr != nullptr)
    {
      delete p_ptr;
      p_ptr = nullptr;
    }
  }
}

template <typename T>
RefCountPtr<T>&
RefCountPtr<T>::operator=(const RefCountPtr<T>& other)
{
  release();
  p_ptr = other.p_ptr;
  p_ref_data = other.p_ref_data;
  lock();
}

} /* furious */ 
