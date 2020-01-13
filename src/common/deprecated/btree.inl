
#include "impl/btree_impl.h"
#include <string.h>
#include <assert.h>
#include <utility>
#include "memory/memory.h"
#include "platform.h"

namespace furious {


template<typename T>
BTree<T>::BTree(mem_allocator_t* allocator) : 
p_root(nullptr),
m_size(0)
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

  p_root = (btree_t*) mem_alloc(&m_allocator, 
                                1, 
                                sizeof(btree_t), 
                                -1);
  *p_root = btree_create();

}

template<typename T>
BTree<T>::~BTree() 
{
  if(p_root != nullptr) 
  {
    BTree<T>::Iterator it = iterator();
    while(it.has_next())
    {
      T* value = it.next().p_value;
      value->~T();
      mem_free(&m_allocator, 
               value);
      m_size--;
    }
    btree_destroy(p_root);
    mem_free(&m_allocator, 
             p_root);
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
    T* value = it.next().p_value;
    value->~T();
    mem_free(&m_allocator, 
             value);
  }
  btree_destroy(p_root);
  mem_free(&m_allocator, 
           p_root);
  p_root = (btree_t*)mem_alloc(&m_allocator, 
                               1, 
                               sizeof(btree_t), 
                               -1);
  *p_root = btree_create();
  m_size = 0;
  
}

template<typename T>
T* 
BTree<T>::insert_copy(uint32_t key, const T* element) 
{
  btree_insert_t insert = btree_insert(p_root, key, nullptr);
  if(insert.m_inserted)
  {
    *insert.p_place = mem_alloc(&m_allocator, 
                                1, 
                                sizeof(T), 
                                -1);
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
  btree_insert_t insert = btree_insert(p_root, key, nullptr);
  if(insert.m_inserted)
  {
    *insert.p_place = mem_alloc(&m_allocator, 
                                1, 
                                sizeof(T), 
                                -1);
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
  T* value = static_cast<T*>(btree_remove(p_root, key));
  if (value != nullptr) 
  {
    value->~T();
    mem_free(&m_allocator, 
             value);
    m_size--;
  }
}

template<typename T>
T*
BTree<T>::get(uint32_t key) const
{
  T* ret =  static_cast<T*>(btree_get(p_root, key));
  return ret;
}

template<typename T>
bool 
BTree<T>::exists(uint32_t key){
  bool res =  get(key) != nullptr;
  return res;
}

template<typename T>
size_t 
BTree<T>::size() const 
{
  size_t ret =  m_size;
  return ret;
}

template<typename T>
typename BTree<T>::Iterator 
BTree<T>::iterator() const
{
  return Iterator(p_root);
}

template<typename T>
BTree<T>::Iterator::Iterator(btree_t* root)
{
  m_iterator = btree_iter_create(root);
}

template<typename T>
BTree<T>::Iterator::~Iterator()
{
  btree_iter_destroy(&m_iterator);
}

template<typename T>
bool 
BTree<T>::Iterator::has_next() const 
{
  return btree_iter_has_next(&m_iterator);
}

template<typename T>
typename BTree<T>::Entry
BTree<T>::Iterator::next() 
{
  btree_entry_t entry = btree_iter_next(&m_iterator);
  return BTree<T>::Entry{entry.m_key, static_cast<T*>(entry.p_value)};
}

} /* furious */ 
