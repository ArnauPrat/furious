

#ifndef _FURIOUS_HASH_MAP_H_
#define _FURIOUS_HASH_MAP_H_

#include "btree.h"

namespace furious
{

template typename<K,V>
struct HashMap
{
  typedef HASH_FUNC uint32_t (*hash_func)(const K* item);
  HashMap(uint32_t );

  void 
  insert(const K* key, T* element);

  T*
  remove(const K* key);

  T*
  get(const K* key);

  bool 
  exists(const K* key);

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
#include "hash_map.inl"
#endif
