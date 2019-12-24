
#include "memory/memory.h"
#include "platform.h"
#include <utility>
#include <new>
#include <stdio.h>

namespace furious
{

#define FURIOUS_DYN_ARRAY_GROWTH_FACTOR 2
#define FURIOUS_DYN_ARRAY_INITIAL_CAPACITY 8

template<typename T>
DynArray<T>::DynArray(mem_allocator_t* allocator) : 
p_data(nullptr),
m_capacity(FURIOUS_DYN_ARRAY_INITIAL_CAPACITY),
m_num_elements(0)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")
  if(allocator != nullptr)
  {
    m_allocator = *allocator; 
  }
  else
  {
    m_allocator = global_mem_allocator;
  }

  p_data = (char*)mem_alloc(&m_allocator, 
                            64, 
                            sizeof(T)*FURIOUS_DYN_ARRAY_INITIAL_CAPACITY, 
                            -1);
}

template<typename T>
DynArray<T>::DynArray(const DynArray& other) : 
p_data(nullptr),
m_capacity(0),
m_num_elements(0)
{
  m_allocator = other.m_allocator;
  p_data = (char*)mem_alloc(&m_allocator, 
                            64, 
                            sizeof(T)*other.m_capacity, -1);
  m_capacity = other.m_capacity;
  m_num_elements = other.m_num_elements;
  for(uint32_t i = 0; i < other.m_num_elements; ++i)
  {
      void* ptr = &(((T*)p_data)[i]);
      new (ptr) T(((T*)other.p_data)[i]);
  }
}

template<typename T>
DynArray<T>::DynArray(DynArray&& other) : 
p_data(nullptr),
m_capacity(0),
m_num_elements(0)

{
  m_allocator = other.m_allocator;
  *this = std::move(other);
}

template<typename T>
DynArray<T>::DynArray(std::initializer_list<T> init) :
p_data(nullptr),
m_capacity(FURIOUS_DYN_ARRAY_INITIAL_CAPACITY),
m_num_elements(0)
{
  m_allocator = global_mem_allocator;
  p_data = (char*)mem_alloc(&m_allocator, 
                            64, 
                            sizeof(T)*FURIOUS_DYN_ARRAY_INITIAL_CAPACITY, 
                            -1);
  for(auto it = init.begin();
      it != init.end();
      ++it)
  {
    this->append(*it);
  }
}

template<typename T>
DynArray<T>::~DynArray()
{
  clear();
  mem_free(&m_allocator, 
           p_data);
}

template <typename T>
void
DynArray<T>::clear()
{
  if(p_data && m_num_elements > 0)
  {
    for(uint32_t i = 0; i < m_num_elements; ++i)
    {
      T* ptr = &(((T*)p_data)[i]);
      ptr->~T();
    }
    m_num_elements = 0;
  }
}

template <typename T>
uint32_t
DynArray<T>::size() const
{
  return m_num_elements;
}

template<typename T>
void
DynArray<T>::append(const T& item)
{
  if(m_num_elements == m_capacity)
  {
    uint32_t new_capacity = m_capacity * FURIOUS_DYN_ARRAY_GROWTH_FACTOR;
    char* temp = (char*)mem_alloc(&m_allocator, 
                                  64, 
                                  sizeof(T)*new_capacity, 
                                  -1);
    for(uint32_t i = 0; i < m_num_elements; ++i)
    {
      void* ptr = temp + i*sizeof(T);
      new (ptr) T(((T*)p_data)[i]);
      (((T*)p_data)[i]).~T();
    }
    mem_free(&m_allocator, 
             p_data);
    p_data = temp;
    m_capacity = new_capacity;
  }

  void* ptr = p_data + m_num_elements*sizeof(T);
  new ( ptr ) T(item);
  m_num_elements++;
}

template<typename T>
void
DynArray<T>::append(T&& item)
{
  if(m_num_elements == m_capacity)
  {
    uint32_t new_capacity = m_capacity * FURIOUS_DYN_ARRAY_GROWTH_FACTOR;
    char* temp = (char*)mem_alloc(&m_allocator, 
                                  64, 
                                  sizeof(T)*new_capacity, 
                                  -1);
    for(uint32_t i = 0; i < m_num_elements; ++i)
    {
      void* ptr = temp + i*sizeof(T); 
      new (ptr) T(std::move(((T*)p_data)[i]));
    }
    mem_free(&m_allocator, 
             p_data);
    p_data = temp;
    m_capacity = new_capacity;
  }

  void* ptr = p_data + m_num_elements*sizeof(T); 
  new ( ptr ) T(std::move(item));
  m_num_elements++;
}

template<typename T>
void
DynArray<T>::append(const DynArray<T>& other)
{
  for(uint32_t i = 0; i < other.size(); ++i)
  {
    this->append(other[i]);
  }
}

template<typename T>
void
DynArray<T>::pop()
{
  if(m_num_elements > 0)
  {
  ((T*)p_data)[m_num_elements-1].~T();
    m_num_elements--;
  }
}

template<typename T>
T&
DynArray<T>::get(uint32_t index)
{
  return ((T*)p_data)[index];
}

template<typename T>
T& 
DynArray<T>::operator[](std::size_t index)
{
  return ((T*)p_data)[index];
}

template<typename T>
const T& 
DynArray<T>::operator[](std::size_t index) const
{
  return ((T*)p_data)[index];
}


template<typename T>
DynArray<T>&
DynArray<T>::operator=(const DynArray& other)
{
  clear();
  mem_free(&m_allocator, 
           p_data);
  p_data = (char*)mem_alloc(&m_allocator, 
                            64, 
                            sizeof(T)*other.m_capacity,
                            -1);
  m_capacity = other.m_capacity;
  m_num_elements = other.m_num_elements;
  for(uint32_t i = 0; i < other.m_num_elements; ++i)
  {
    void* ptr = p_data + i*sizeof(T);
    new (ptr) T(((T*)other.p_data)[i]);
  }
  return *this;
}

template<typename T>
DynArray<T>&
DynArray<T>::operator=(DynArray&& other)
{
  clear();
  mem_free(&m_allocator, 
           p_data);
  p_data = other.p_data;
  m_capacity = other.m_capacity;
  m_num_elements = other.m_num_elements;
  other.p_data = nullptr;
  other.m_capacity = 0;
  other.m_num_elements = 0;
  return *this;
}

template<typename T>
T*
DynArray<T>::buffer() const
{
  return (T*)p_data;
}
  
}  
