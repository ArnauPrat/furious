#include "../../common/utils.h"
namespace furious 
{

template<typename T>
  void destructor(void *ptr) 
  {
    static_cast<T*>(ptr)->~T();
  }


template <typename T>
TableView<T> 
Database::create_table(const std::string& tablename) 
{
  
  uint32_t hash_value = get_table_id<T>(tablename);
  assert(m_tables.get(hash_value) == nullptr);
  Table* table = m_tables.get(hash_value);
  if(table != nullptr)
  {
    return TableView<T>(nullptr);
  }
  table =  new Table(tablename, hash_value, sizeof(T), &destructor<T>);
  m_tables.insert(hash_value,table);
  return TableView<T>(table); 
}

template <typename T>
void 
Database::remove_table(const std::string& tablename) 
{
  uint32_t hash_value = get_table_id<T>(tablename);
  Table* table = m_tables.get(hash_value);
  assert(table != nullptr);
  delete table;
  m_tables.remove(hash_value);
}

template <typename T>
uint32_t
Database::get_table_id(const std::string& tablename)
{
  return hash(tablename);
}


template <typename T>
TableView<T> 
Database::find_table(const std::string& tablename) 
{
  uint32_t hash_value = get_table_id<T>(tablename);
  Table* table = m_tables.get(hash_value);
  assert(table != nullptr);
  return TableView<T>(static_cast<Table*>(table));
}

template <typename T>
TableView<T> 
Database::find_or_create_table(const std::string& tablename)
{
  uint32_t hash_value = get_table_id<T>(tablename); 
  Table* table = m_tables.get(hash_value);
  if(table != nullptr) {
    return TableView<T>(table);
  }
  table =  new Table(tablename, hash_value, sizeof(T), &destructor<T>);
  m_tables.insert(hash_value,table);
  return TableView<T>(table); 
}

template <typename T>
bool Database::exists_table(const std::string& tablename) 
{
  uint32_t table_id = get_table_id<T>(tablename);
  return m_tables.get(table_id) != nullptr;
}

} /* furious */ 
