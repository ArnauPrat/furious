


namespace furious
{

template <typename K, typename V>
void 
HashMap<K,V>::insert(const K* key, V* element)
{
  m_btree.insert(HASH_FUNC(key), element);
}

template <typename K, typename V>
V*
HashMap<K,V>::remove(const K* key)
{
  return m_btree.remove(HASH_FUNC(key));
}

template <typename K, typename V>
V*
HashMap<K,V>::get(const K* key)
{
  return m_btree.get(HASH_FUNC(key));
}

template <typename K, typename V>
bool 
HashMap<K,V>::exists(const K* key)
{
  return m_btree.exists(HASH_FUNC(key));
}

template <typename K, typename V>
size_t
HashMap<K,V>::size()
{
  return m_btree.size();
}

template <typename K, typename V>
void 
HashMap<K,V>::clear()
{
  m_btree.clear();
}
}
