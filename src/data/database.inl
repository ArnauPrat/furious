namespace furious {

template<typename T>
  void destructor(void *ptr) {
    static_cast<T*>(ptr)->~T();
  }
  
template <typename T>
TableView<T> Database::add_table() {
  assert(m_tables.find(hash(typeid(T).name())) == m_tables.end());
  int64_t hash_value = get_table_id<T>(); 
  if(m_tables.find(hash_value) != m_tables.end()) {
    return nullptr;
  }
  std::string table_name = type_name<T>();
  auto table =  new Table(table_name, hash_value, sizeof(T), &destructor<T>);
  m_tables.insert(std::make_pair(hash_value,table));
  return TableView<T>(table); 
}

template <typename T>
void Database::remove_table() {
  int64_t hash_value = get_table_id<T>();
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  delete table->second;
  m_tables.erase(table);
}

template <typename T>
TableView<T> Database::find_table() {
  int64_t hash_value = get_table_id<T>();
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  return TableView<T>(static_cast<Table*>(table->second));
}

template <typename T>
int64_t Database::get_table_id() {
  return get_table_id(type_name<T>());
}

template <typename T>
bool Database::exists_table() {
  return exists_table(type_name<T>());
}

} /* furious */ 
