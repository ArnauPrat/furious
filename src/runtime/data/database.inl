
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
Database::create_table() 
{
  uint32_t hash_value = get_table_id<T>();
  assert(m_tables.get(hash_value) == nullptr);
  if(m_tables.get(hash_value) != nullptr)
  {
    return TableView<T>(nullptr);
  }
  Table* table =  new Table(T::_component_name(), hash_value, sizeof(T), &destructor<T>);
  m_tables.insert_copy(hash_value,&table);
  return TableView<T>(table); 
}

template <typename T>
void 
Database::remove_table() 
{
  uint32_t hash_value = get_table_id<T>();
  Table** table = m_tables.get(hash_value);
  if(table != nullptr)
  {
    delete *table;
    m_tables.remove(hash_value);
  }
}

template <typename T>
uint32_t
Database::get_table_id()
{
  return hash(T::_component_name());
}


template <typename T>
TableView<T> 
Database::find_table() 
{
  uint32_t hash_value = get_table_id<T>();
  Table* table = *m_tables.get(hash_value);
  assert(table != nullptr);
  return TableView<T>(static_cast<Table*>(table));
}

template <typename T>
TableView<T> 
Database::find_or_create_table()
{
  uint32_t hash_value = get_table_id<T>(); 
  Table** table = m_tables.get(hash_value);
  if(table != nullptr) {
    return TableView<T>(*table);
  }
  Table* table_ptr =  new Table(T::_component_name(), hash_value, sizeof(T), &destructor<T>);
  m_tables.insert_copy(hash_value,&table_ptr);
  return TableView<T>(table_ptr); 
}

template <typename T>
bool Database::exists_table() 
{
  uint32_t table_id = get_table_id<T>();
  return m_tables.get(table_id) != nullptr;
}

template <typename T>
TableView<T>
Database::create_temp_table(const std::string& table_name)
{
  uint32_t hash_value = hash(table_name.c_str()); 
  Table** table = m_temp_tables.get(hash_value);
  if(table != nullptr) {
    return TableView<T>(*table);
  }
  Table* table_ptr =  new Table(table_name, hash_value, sizeof(T), nullptr);
  m_temp_tables.insert_copy(hash_value,&table_ptr);
  return TableView<T>(table_ptr); 
}

template <typename T>
void
Database::remove_temp_table(TableView<T> table_view)
{
  uint32_t hash_value = hash(table_view.get_raw()->m_name);
  Table** table = m_temp_tables.get(hash_value);
  if(table != nullptr)
  {
    delete *table;
    m_temp_tables.remove(hash_value);
  }
}

} /* furious */ 
