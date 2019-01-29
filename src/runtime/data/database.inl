namespace furious {

template<typename T>
  void destructor(void *ptr) 
  {
    static_cast<T*>(ptr)->~T();
  }


template <typename T>
TableView<T> 
Database::create_table(const std::string& tablename) 
{
  assert(m_tables.find(get_table_id<T>(tablename)) == m_tables.end());
  int64_t hash_value = get_table_id<T>(tablename); 
  if(m_tables.find(hash_value) != m_tables.end()) {
    return TableView<T>(nullptr);
  }
  auto table =  new Table(tablename, hash_value, sizeof(T), &destructor<T>);
  m_tables.insert(std::make_pair(hash_value,table));
  return TableView<T>(table); 
}

template <typename T>
void 
Database::remove_table(const std::string& tablename) 
{
  int64_t hash_value = get_table_id<T>(tablename);
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  delete table->second;
  m_tables.erase(table);
}

template <typename T>
int64_t
Database::get_table_id(const std::string& tablename)
{
  return hash(tablename);
}


template <typename T>
TableView<T> Database::find_table(const std::string& tablename) 
{
  int64_t hash_value = get_table_id<T>(tablename);
  auto table = m_tables.find(hash_value);
  assert(table != m_tables.end());
  return TableView<T>(static_cast<Table*>(table->second));
}

template <typename T>
TableView<T> Database::find_or_create_table(const std::string& tablename)
{
  int64_t hash_value = get_table_id<T>(tablename); 
  auto it = m_tables.find(hash_value);
  if(it != m_tables.end()) {
    return TableView<T>{it->second};
  }
  std::string table_name = type_name<T>::name();
  auto table =  new Table(table_name, hash_value, sizeof(T), &destructor<T>);
  m_tables.insert(std::make_pair(hash_value,table));
  return TableView<T>(table); 
}

template <typename T>
bool Database::exists_table(const std::string& tablename) 
{
  int64_t table_id = get_table_id<T>(tablename);
  return m_tables.find(table_id) != m_tables.end();
}

} /* furious */ 
