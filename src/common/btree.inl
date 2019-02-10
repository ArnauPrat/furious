
#include "impl/btree_impl.h"
#include <string.h>
#include <assert.h>
#include <utility>

namespace furious {


template<typename T>
BTree<T>::BTree() : p_root(btree_create_root()),
                 m_size(0)
{

}

template<typename T>
BTree<T>::~BTree() 
{
  if(p_root != nullptr) 
  {
    BTree<T>::Iterator it = iterator();
    while(it.has_next())
    {
      T* value = (T*)it.next();
      value->~T();
      free(value);
      m_size--;
    }
    btree_destroy_root(p_root);
    p_root = nullptr;
  }
}

template<typename T>
void 
BTree<T>::clear() 
{
  BTree<T>::Iterator it = iterator();
  while(it.has_next())
  {
    T* value = (T*)it.next();
    value->~T();
    free(value);
  }
  btree_destroy_root(p_root);
  p_root = btree_create_root();
  m_size = 0;
}

template<typename T>
T* 
BTree<T>::insert_copy(uint32_t key, const T* element) 
{
  BTInsert insert = btree_insert_root(p_root, key);
  if(insert.m_inserted)
  {
    *insert.p_place = malloc(sizeof(T));
    new (*insert.p_place) T(*element);
    m_size++;
    return (T*)*insert.p_place;
  }
  return nullptr;
}

template<typename T>
template <typename...Args>
T* 
BTree<T>::insert_new(uint32_t key, Args &&... args)
{
  BTInsert insert = btree_insert_root(p_root, key);
  if(insert.m_inserted)
  {
    *insert.p_place = malloc(sizeof(T));
    new (*insert.p_place) T(std::forward<Args>(args)...);
    m_size++;
    return (T*)*insert.p_place;
  }
  return nullptr;
}

template<typename T>
void
BTree<T>::remove(uint32_t key) 
{
  T* value = static_cast<T*>(btree_remove_root(p_root, key));
  if (value != nullptr) 
  {
    value->~T();
    free(value);
    m_size--;
  }
}

template<typename T>
T*
BTree<T>::get(uint32_t key) const
{
  return static_cast<T*>(btree_get_root(p_root, key));
}

template<typename T>
bool 
BTree<T>::exists(uint32_t key){
  return get(key) != nullptr;
}

template<typename T>
size_t 
BTree<T>::size() {
  return m_size;
}

template<typename T>
typename BTree<T>::Iterator 
BTree<T>::iterator() const
{
  return Iterator(p_root);
}

template<typename T>
BTree<T>::Iterator::Iterator(BTRoot* root) : 
m_iterator(root) 
{
}

template<typename T>
bool 
BTree<T>::Iterator::has_next() const 
{
  return m_iterator.has_next();
}

template<typename T>
T* 
BTree<T>::Iterator::next() {
  return static_cast<T*>(m_iterator.next());
}

} /* furious */ 
