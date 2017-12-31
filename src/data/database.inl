namespace furious {

template<typename T>
  void destructor(void *ptr) {
    static_cast<T*>(ptr)->~T();
  }
  
template <typename T>
TableView<T> Database::create_table() {
  assert(m_tables.find(hash(typeid(T).name())) == m_tables.end());
  uint64_t hash_value = hash(type_name<T>()); 
  if(m_tables.find(hash_value) != m_tables.end()) {
    return nullptr;
  }
  auto table =  new Table(type_name<T>(), sizeof(T), &destructor<T>);
  m_tables.insert(std::make_pair(hash_value,table));
  return TableView<T>(table); 
}

template <typename T>
void Database::remove_table() {
  uint64_t hash_value = hash(type_name<T>());
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  delete table->second;
  m_tables.erase(table);
}

template <typename T>
TableView<T> Database::find_table() {
  uint64_t hash_value = hash(type_name<T>());
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  return TableView<T>(static_cast<Table*>(table->second));
}

} /* furious */ 
