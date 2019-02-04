

#ifndef _FURIOUS_HASH_MAP_H_
#define _FURIOUS_HASH_MAP_H_

#include "btree.h"

namespace furious
{

template <typename K, typename V>
struct HashMap
{
  typedef uint32_t (*HASH_FUNC)(const K* item);
  HashMap() = default;
  ~HashMap() = default;

  void 
  insert(const K* key, V* element);

  V*
  remove(const K* key);

  V*
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
