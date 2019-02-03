


namespace furious
{

template typename<K,V>
void 
HashMap<K,V>::insert(const K* key, V* element)
{
  m_btree.insert(HASH_FUNC(key), element);
}

template typename<K,V>
V*
HashMap<K,V>::remove(const K* key)
{
  return m_btree.remove(HASH_FUNC(key));
}

template typename<K,V>
V*
HashMap<K,V>::get(const K* key)
{
  return m_btree.get(HASH_FUNC(key));
}

template typename<K,V>
bool 
HashMap<K,V>::exists(const K* key)
{
  return m_btree.exists(HASH_FUNC(key));
}

template typename<K,V>
size_t
HashMap<K,V>::size()
{
  m_btree.size();
}

template typename<K,V>
void 
HashMap<K,V>::clear()
{
  m_btree.clear();
}
}
