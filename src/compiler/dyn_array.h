

#ifndef _FURIOUS_DYNAMIC_ARRAY_H_
#define _FURIOUS_DYNAMIC_ARRAY_H_

#include "../common/platform.h"
#include "../common/memory/memory.h"

#include <initializer_list>
#include <stdlib.h>

template <typename T>
struct DynArray
{
  DynArray(fdb_mem_allocator_t* allocator = nullptr);
  DynArray(const DynArray&);
  DynArray(DynArray&&);
  DynArray(std::initializer_list<T>);

  ~DynArray();

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  uint32_t
  size() const;

  void
  clear();

  //void
  //reset();

  void
  append(const T&);

  void
  append(const DynArray<T>&);

  void
  append(T&&);

  void
  pop();

  T&
  get(uint32_t index);

  T& 
  operator[](std::size_t index);

  const T& 
  operator[](std::size_t index) const;

  DynArray<T>&
  operator=(const DynArray&);

  DynArray<T>&
  operator=(DynArray&&);

  T*
  buffer() const;

private:


  char*           p_data;
  uint32_t        m_capacity;
  uint32_t        m_num_elements;
  fdb_mem_allocator_t m_allocator;
};
  
#endif

#include "dyn_array.inl"
