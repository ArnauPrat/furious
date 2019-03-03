

#ifndef _FURIOUS_REFCOUNT_PTR_H_
#define _FURIOUS_REFCOUNT_PTR_H_

#include "types.h"


namespace furious
{

struct RefData
{
  RefData();
  uint32_t  m_num_refs;
};

template <typename T>
struct RefCountPtr
{
  RefCountPtr(T* ptr);
  RefCountPtr(const RefCountPtr<T>& other);
  ~RefCountPtr();

  T*
  get() const;

  void
  lock();

  void
  release();

  RefCountPtr<T>&
  operator=(const RefCountPtr<T>& other);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  T*        p_ptr;
  RefData*  p_ref_data;
  
};
  
} /* furious */ 

#include "refcount_ptr.inl"
#endif
