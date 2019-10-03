
#include "../../common/utils.h"
#include <utility>

namespace furious 
{



template <typename T, void (*fp)(T*)>
TableView<T> 
Database::create_table() 
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  assert(m_tables.get(hash_value) == nullptr);
  if(m_tables.get(hash_value) != nullptr)
  {
    release();
    return TableView<T>(nullptr);
  }

  Table* table = new Table(T::_component_name(), hash_value, sizeof(T), destructor_manual<T,fp>);
  m_tables.insert_copy(hash_value,&table);
  release();
  return TableView<T>(table); 
}

template <typename T>
TableView<T> 
Database::create_table() 
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  assert(m_tables.get(hash_value) == nullptr);
  if(m_tables.get(hash_value) != nullptr)
  {
    release();
    return TableView<T>(nullptr);
  }

  Table* table = new Table(T::_component_name(), hash_value, sizeof(T), destructor<T>);
  m_tables.insert_copy(hash_value,&table);
  release();
  return TableView<T>(table); 
}

template <typename T>
void 
Database::remove_table() 
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  Table** table = m_tables.get(hash_value);
  if(table != nullptr)
  {
    delete *table;
    m_tables.remove(hash_value);
  }
  release();
  return;
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
  lock();
  uint32_t hash_value = get_table_id<T>();
  Table* table = *m_tables.get(hash_value);
  assert(table != nullptr);
  release();
  return TableView<T>(static_cast<Table*>(table));
}

template <typename T>
TableView<T> 
Database::find_or_create_table()
{
  lock();
  uint32_t hash_value = get_table_id<T>(); 
  Table** table = m_tables.get(hash_value);
  if(table != nullptr) 
  {
    release();
    return TableView<T>(*table);
  }
  Table* table_ptr =  new Table(T::_component_name(), hash_value, sizeof(T), destructor<T>);
  m_tables.insert_copy(hash_value,&table_ptr);
  release();
  return TableView<T>(table_ptr); 
}

template <typename T, void (*fp)(T*)>
TableView<T> 
Database::find_or_create_table()
{
  lock();
  uint32_t hash_value = get_table_id<T>(); 
  Table** table = m_tables.get(hash_value);
  if(table != nullptr) 
  {
    release();
    return TableView<T>(*table);
  }
  Table* table_ptr =  new Table(T::_component_name(), hash_value, sizeof(T), destructor_manual<T,fp>);
  m_tables.insert_copy(hash_value,&table_ptr);
  release();
  return TableView<T>(table_ptr); 
}

template <typename T>
bool Database::exists_table() 
{
  lock();
  uint32_t table_id = get_table_id<T>();
  bool res = m_tables.get(table_id) != nullptr;
  release();
  return res;
}

template <typename T>
TableView<T>
Database::create_temp_table(const char* table_name)
{
  lock();
  TableView<T> res = create_temp_table_no_lock<T>(table_name);
  release();
  return res;
}

template <typename T>
TableView<T>
Database::create_temp_table_no_lock(const char* table_name)
{
  uint32_t hash_value = hash(table_name); 
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
  lock();
  remove_temp_table<T>(table_view);
  release();
}

template <typename T>
void
Database::remove_temp_table_no_lock(TableView<T> table_view)
{
  uint32_t hash_value = hash(table_view.get_raw()->m_name);
  Table** table = m_temp_tables.get(hash_value);
  if(table != nullptr)
  {
    delete *table;
    m_temp_tables.remove(hash_value);
  }
}

template <typename T,typename...Args>
T*
Database::create_global(Args...args)
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  assert(m_globals.get(hash_value) == nullptr);
  if(m_globals.get(hash_value) != nullptr)
  {
    release();
    return nullptr;
  }
  GlobalInfo g_info;
  char* buffer = new char[sizeof(T)];
  g_info.p_global = new (buffer) T{std::forward<Args>(args)...};
  g_info.m_destructor = destructor<T>;
  m_globals.insert_copy(hash_value,&g_info);
  release();
  return (T*)g_info.p_global; 
}

template <typename T>
void 
Database::remove_global()
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  GlobalInfo* global = m_globals.get(hash_value);
  if(global != nullptr)
  {
    global->m_destructor(global->p_global);
    delete [] ((char*)global->p_global); 
    m_globals.remove(hash_value);
  }
  release();
  return;
}

template <typename T>
T* 
Database::find_global()
{
  lock();
  T* global = find_global_no_lock<T>();
  release();
  return global;
}

template <typename T>
T* 
Database::find_global_no_lock()
{
  uint32_t hash_value = get_table_id<T>();
  GlobalInfo* global = m_globals.get(hash_value);
  assert(global != nullptr);
  return (T*)global->p_global;
}

template <typename T,typename...Args>
T* 
Database::find_or_create_global(Args...args)
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  GlobalInfo* global = m_globals.get(hash_value); 
  if(global != nullptr)
  {
    T temp = T(std::forward<Args>(args)...);
    *((T*)global->p_global) = temp;
    release();
    return global;
  }
  GlobalInfo g_info;
  char* buffer = new char[sizeof(T)];
  g_info.p_global = new(buffer) T{std::forward<Args>(args)...};
  g_info.m_destructor = destructor<T>;
  m_globals.insert_copy(hash_value,&g_info);
  release();
  return g_info.p_global; 
}

template<typename T>
bool 
Database::exists_global()
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  bool res = m_globals.get(hash_value) != nullptr;
  release();
  return res;
}

template<typename T>
void
Database::add_refl_data(RefCountPtr<ReflData> refl_strct)
{
  assert(strcmp(T::_component_name(), refl_strct.get()->m_type_name) == 0);
  lock();
  uint32_t hash_value = get_table_id<T>();
  m_refl_data.insert_copy(hash_value, &refl_strct);
  release();
  return;
}

template<typename T>
const ReflData* 
Database::get_refl_data()
{
  lock();
  uint32_t hash_value = get_table_id<T>();
  RefCountPtr<ReflData>* data = m_refl_data.get(hash_value);
  ReflData* ret = nullptr;
  if(data != nullptr)
  {
    ret = data->get();
  }
  release();
  return ret;
}


} /* furious */ 
