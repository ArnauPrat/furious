

#ifndef _FURIOUS_HASH_TREE_H_
#define _FURIOUS_HASH_TREE_H_

#include "btree.h"

namespace furious
{

template typename<K,V>
struct HashDict
{
  typedef HASH_FUNC uint32_t (*hash_func)(const K& item);
  HashDict(uint32_t );

  void 
  insert(uint32_t key, T* element);

  T*
  remove(uint32_t key);

  T*
  get(uint32_t key);

  bool 
  exists(uint32_t key);

  size_t
  size();

  void 
  clear();

private:
  BTree<V>  m_btree;
  HASH_FUNC p_hash_func;
};
  
} /* furious
 */ 
#include "hash_dict.inl"
#endif
