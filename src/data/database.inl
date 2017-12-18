namespace furious {

template<typename T>
  void destructor(void *ptr) {
    static_cast<T*>(ptr)->~T();
  }
  
template <typename T>
TableView<T> Database::create_table() {
  assert(m_tables.find(type_name<T>()) == m_tables.end());
  if(m_tables.find(type_name<T>()) != m_tables.end()) {
    return nullptr;
  }
  auto table =  new Table(type_name<T>(), sizeof(T), &destructor<T>);
  m_tables.insert(TableMapPair(table->table_name(),table));
  return TableView<T>(table); 
}

template <typename T>
void Database::remove_table() {
  auto table = m_tables.find(type_name<T>());
  assert(table != m_tables.end());
  delete table->second;
  m_tables.erase(table);
}

template <typename T>
TableView<T> Database::find_table() {
  auto table = find_table(type_name<T>());
  assert(table != nullptr);
  return TableView<T>(static_cast<Table*>(table));
}

} /* furious */ 
