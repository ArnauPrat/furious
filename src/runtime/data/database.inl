
#ifndef _FURIOUS_DATABASE_INL_
#define _FURIOUS_DATABASE_INL_

#include "../../common/utils.h"
#include <string.h>

namespace furious 
{

template <typename T, void (*fp)(T*)>
TableView<T> 
database_create_table(database_t* database) 
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  FURIOUS_ASSERT(btree_get(&database->m_tables, hash_value) == nullptr && "Table already exists");
  if(btree_get(&database->m_tables, hash_value) != nullptr)
  {
    database_release(database);
    return TableView<T>(nullptr);
  }

  table_t* table = (table_t*)mem_alloc(&database->m_table_allocator, 
                                       FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                       sizeof(table_t), 
                                       FURIOUS_NO_HINT);;

  *table = table_create(T::__component_name(), 
                          hash_value, 
                          sizeof(T), 
                          destructor_manual<T,fp>);

  btree_insert(&database->m_tables, 
               hash_value, 
               table);

  database_release(database);
  return TableView<T>(table); 
}

template <typename T>
TableView<T> 
database_create_table(database_t* database) 
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  FURIOUS_ASSERT(btree_get(&database->m_tables, hash_value) == nullptr && "Table already exists");
  if(btree_get(&database->m_tables, hash_value) != nullptr)
  {
    database_release(database);
    return TableView<T>(nullptr);
  }

  table_t* table = (table_t*)mem_alloc(&database->m_table_allocator, 
                                       FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                       sizeof(table_t), 
                                       FURIOUS_NO_HINT);;

  *table = table_create(T::__component_name(), 
                          hash_value, 
                          sizeof(T), 
                          destructor<T>);
  btree_insert(&database->m_tables, hash_value, table);
  database_release(database);
  return TableView<T>(table); 
}




template <typename T>
TableView<T> 
database_find_table(database_t* database) 
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  table_t* table = (table_t*)btree_get(&database->m_tables, hash_value);
  FURIOUS_ASSERT(table != nullptr && "Find on a null table");
  database_release(database);
  return TableView<T>(table);
}

template <typename T>
TableView<T> 
database_find_or_create_table(database_t* database)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name()); 
  table_t* table = (table_t*)btree_get(&database->m_tables, hash_value);
  if(table != nullptr) 
  {
    database_release(database);
    return TableView<T>(table);
  }
  table = (table_t*)mem_alloc(&database->m_table_allocator, 
                              FURIOUS_DATABASE_TABLE_ALIGNMENT,
                              sizeof(table_t), 
                              FURIOUS_NO_HINT);;

  *table = table_create(T::__component_name(), 
                        hash_value, 
                        sizeof(T), 
                        destructor<T>);
  btree_insert(&database->m_tables, hash_value, table);
  database_release(database);
  return TableView<T>(table); 
}

template <typename T, void (*fp)(T*)>
TableView<T> 
database_find_or_create_table(database_t* database)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name()); 
  table_t* table = (table_t*)btree_get(&database->m_tables, hash_value);
  if(table != nullptr) 
  {
    database_release(database);
    return TableView<T>(*table);
  }
  table = (table_t*)mem_alloc(&database->m_table_allocator, 
                              FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                              sizeof(table_t), 
                              FURIOUS_NO_HINT);;

  *table = table_create(T::__component_name(), 
                            hash_value, 
                            sizeof(T), 
                            destructor_manual<T,fp>);
  btree_insert(&database->m_tables, hash_value, table);
  database_release(database);
  return TableView<T>(table); 
}

template <typename T>
bool database_exists_table(database_t* database) 
{
  database_lock(database);
  uint32_t table_id = get_table_id(T::__component_name());
  bool res = btree_get(&database->m_tables, table_id) != nullptr;
  database_release(database);
  return res;
}


template <typename T>
TableView<T>
database_create_temp_table(database_t* database,
                           const char* table_name)
{
  database_lock(database);
  TableView<T> res = database_create_temp_table_no_lock<T>(database, table_name);
  database_release(database);
  return res;
}

template <typename T>
TableView<T>
database_create_temp_table_no_lock(database_t* database,
                                   const char* table_name)
{
  uint32_t hash_value = hash(table_name); 
  table_t* table = (table_t*)btree_get(&database->m_temp_tables, 
                                       hash_value);
  if(table != nullptr) {
    return TableView<T>(table);
  }
  table = (table_t*)mem_alloc(&database->m_table_allocator, 
                              FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                              sizeof(table_t), 
                              FURIOUS_NO_HINT);;

  *table = table_create(table_name, 
                        hash_value, 
                        sizeof(T), 
                        nullptr);
  btree_insert(&database->m_temp_tables, hash_value, table);
  return TableView<T>(table); 
}

template <typename T>
void
database_remove_temp_table(database_t* database,
                           TableView<T> table_view)
{
  database_lock(database);
  database_remove_temp_table_no_lock<T>(database, 
                               table_view);
  database_release(database);
}

template <typename T>
void
database_remove_temp_table_no_lock(database_t* database,
                                   TableView<T> table_view)
{
  uint32_t hash_value = hash(table_view.get_raw()->m_name);
  table_t* table = (table_t*)btree_get(&database->m_temp_tables, hash_value);
  if(table != nullptr)
  {
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
    btree_remove(&database->m_temp_tables, hash_value);
  }
}

template <typename T,typename...Args>
T*
database_create_global(database_t* database,
                       Args...args)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  GlobalInfo* global = (GlobalInfo*)btree_get(&database->m_globals, hash_value);
  FURIOUS_ASSERT(global == nullptr && "Global already exists");
  if(global != nullptr)
  {
    database_release(database);
    return nullptr;
  }
  global = (GlobalInfo*)mem_alloc(&database->m_global_allocator, 
                                  64, 
                                  sizeof(GlobalInfo), 
                                  FURIOUS_NO_HINT);

  void* buffer = mem_alloc(&database->m_global_allocator, 
                           64, 
                           sizeof(T), 
                           FURIOUS_NO_HINT);

  global->p_global = new (buffer) T{std::forward<Args>(args)...};
  global->m_destructor = destructor<T>;
  btree_insert(&database->m_globals, hash_value, global);
  database_release(database);
  return (T*)global->p_global; 
}


template <typename T>
T* 
database_find_global(database_t* database)
{
  database_lock(database);
  T* global = database_find_global_no_lock<T>(database);
  database_release(database);
  return global;
}

template <typename T>
T* 
database_find_global_no_lock(database_t* database)
{
  uint32_t hash_value = get_table_id(T::__component_name());
  GlobalInfo* global = (GlobalInfo*)btree_get(&database->m_globals, hash_value);
  if(global == nullptr)
  {
    return nullptr;
  }
  return (T*)global->p_global;
}

template <typename T,typename...Args>
T* 
database_find_or_create_global(database_t* database, 
                               Args...args)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  GlobalInfo* global = (GlobalInfo*)btree_get(&database->m_globals, hash_value); 
  if(global != nullptr)
  {
    T temp = T(std::forward<Args>(args)...);
    *((T*)global->p_global) = temp;
    database_release(database);
    return global;
  }
  global = (GlobalInfo*)mem_alloc(&database->m_global_allocator, 
                                  64, 
                                  sizeof(GlobalInfo), 
                                  FURIOUS_NO_HINT);

  void* buffer = mem_alloc(&database->m_global_allocator, 
                           64, 
                           sizeof(T), 
                           FURIOUS_NO_HINT);

  global->p_global = new(buffer) T{std::forward<Args>(args)...};
  global->m_destructor = destructor<T>;
  btree_insert(&database->m_globals, hash_value, global);
  database_release(database);
  return global->p_global; 
}

template <typename T>
void
database_add_refl_data(database_t* database,
                       RefCountPtr<ReflData> refl_strct)
{
  FURIOUS_ASSERT(strcmp(T::__component_name(), refl_strct.get()->m_type_name) == 0);
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  RefCountPtr<ReflData>* refl_cpy = new RefCountPtr<ReflData>();
  *refl_cpy = refl_strct;
  btree_insert(&database->m_refl_data, hash_value, refl_cpy);
  database_release(database);
  return;
}

template <typename T>
const ReflData* 
database_get_refl_data(database_t* database)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  RefCountPtr<ReflData>* data = (RefCountPtr<ReflData>*)btree_get(&database->m_refl_data, hash_value);
  ReflData* ret = nullptr;
  if(data != nullptr)
  {
    ret = data->get();
  }
  database_release(database);
  return ret;
}

template <typename T>
void 
database_remove_global(database_t* database)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  GlobalInfo* global = (GlobalInfo*)btree_get(&database->m_globals, hash_value);
  if(global != nullptr)
  {
    global->m_destructor(global->p_global);
    mem_free(&database->m_global_allocator, global->p_global); 
    mem_free(&database->m_global_allocator, global); 
    btree_remove(&database->m_globals, hash_value);
  }
  database_release(database);
  return;
}

template <typename T>
bool 
database_exists_global(database_t* database)
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  bool res = btree_get(&database->m_globals, hash_value) != nullptr;
  database_release(database);
  return res;
}

template <typename T>
void 
database_remove_table(database_t* database) 
{
  database_lock(database);
  uint32_t hash_value = get_table_id(T::__component_name());
  table_t* table = (table_t*)btree_get(&database->m_tables, hash_value);
  if(table != nullptr)
  {
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
    btree_remove(&database->m_tables, hash_value);
  }
  database_release(database);
  return;
}

} /* furious */ 
#endif
