

#ifndef _FURIOUS_DYNAMIC_ARRAY_H_
#define _FURIOUS_DYNAMIC_ARRAY_H_

#include "types.h"
#include "stdlib.h"

namespace furious
{

template <typename T>

struct DynArray
{
  DynArray();
  ~DynArray();

  DynArray(const DynArray&);

  uint32_t
  size() const;

  void
  clear();

  void
  append(const T&);

  void
  pop();

  void
  append(T&&);

  T&
  get(uint32_t index);

  T& 
  operator[](std::size_t index);

  const T& 
  operator[](std::size_t index) const;


private:
  T*        p_data;
  uint32_t  m_capacity;
  uint32_t  m_num_elements;

};
  
} /* furious
 */ 

#include "dyn_array.inl"

#endif
