
#include "impl/btree_impl.h"
#include <string.h>
#include <cassert>

namespace furious {


template<typename T>
BTree<T>::BTree() : p_root(btree_create_internal()),
                 m_size(0)
{

}

template<typename T>
BTree<T>::~BTree() {
  if(p_root != nullptr) {
    btree_destroy_node(p_root);
    p_root = nullptr;
  }
}

template<typename T>
void BTree<T>::clear() {
  btree_destroy_node(p_root);
  p_root = btree_create_internal();
  m_size = 0;
}

template<typename T>
void BTree<T>::insert(uint8_t key, T* element) {
  m_size++;
  btree_insert_root(&p_root, key, static_cast<void*>(element));
}

template<typename T>
T* BTree<T>::remove(uint8_t key) {
  T* value = static_cast<T*>(btree_remove(&p_root, key));
  if (value != nullptr) {
    m_size--;
  }
  return value;
}

template<typename T>
T* BTree<T>::get(uint8_t key){
  return static_cast<T*>(btree_get(p_root, key));
}

template<typename T>
bool BTree<T>::exists(uint8_t key){
  return get(key) != nullptr;
}

template<typename T>
size_t BTree<T>::size() {
  return m_size;
}

template<typename T>
typename BTree<T>::Iterator* BTree<T>::iterator() {
  return new Iterator{p_root};
}

template<typename T>
BTree<T>::Iterator::Iterator(BTNode* root) : m_iterator(new BTIterator{root}) {
}

template<typename T>
BTree<T>::Iterator::~Iterator() {
  delete m_iterator;
}

template<typename T>
bool BTree<T>::Iterator::has_next() {
  return m_iterator->has_next();
}

template<typename T>
T* BTree<T>::Iterator::next() {
  return static_cast<T*>(m_iterator->next());
}

} /* furious */ 
