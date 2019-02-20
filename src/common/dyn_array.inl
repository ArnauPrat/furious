
#include <utility>
namespace furious
{

#define _FURIOUS_DYN_ARRAY_GROWTH_FACTOR 8

template<typename T>
DynArray<T>::DynArray() : 
p_data(nullptr),
m_capacity(0),
m_num_elements(0)
{
}

template<typename T>
DynArray<T>::DynArray(const DynArray& other)
{
  p_data = new T[other.m_capacity];
  m_capacity = other.m_capacity;
  m_num_elements = other.m_num_elements;
  for(uint32_t i = 0; i < other.m_num_elements; ++i)
  {
    p_data[i] = other.p_data[i];
  }
}

template<typename T>
DynArray<T>::~DynArray()
{
  clear();
}

template <typename T>
void
DynArray<T>::clear()
{
  if(p_data)
  {
    delete [] p_data;
    p_data = nullptr;
    m_capacity = 0;
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
    uint32_t new_capacity = m_capacity + _FURIOUS_DYN_ARRAY_GROWTH_FACTOR;
    T* temp = new T[new_capacity];
    for(uint32_t i = 0; i < m_num_elements; ++i)
    {
      temp[i] = p_data[i];
    }
    delete [] p_data;
    p_data = temp;
    m_capacity = new_capacity;
  }

  p_data[m_num_elements] = item;
  m_num_elements++;
}

template<typename T>
void
DynArray<T>::append(T&& item)
{
  if(m_num_elements == m_capacity)
  {
    uint32_t new_capacity = m_capacity + _FURIOUS_DYN_ARRAY_GROWTH_FACTOR;
    T* temp = new T[new_capacity];
    for(uint32_t i = 0; i < m_num_elements; ++i)
    {
      temp[i] = std::move(p_data[i]);
    }
    delete [] p_data;
    p_data = temp;
    m_capacity = new_capacity;
  }

  p_data[m_num_elements] = std::move(item);
  m_num_elements++;
}

template<typename T>
void
DynArray<T>::pop()
{
  if(m_num_elements > 0)
  {
    m_num_elements--;
  }
}

template<typename T>
T&
DynArray<T>::get(uint32_t index)
{
  return p_data[index];
}

template<typename T>
T& 
DynArray<T>::operator[](std::size_t index)
{
  return p_data[index];
}

template<typename T>
const T& 
DynArray<T>::operator[](std::size_t index) const
{
  return p_data[index];
}

  
}  
