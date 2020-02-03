
#include "../../common/utils.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/memory/stack_allocator.h"
#include "bittable.h"
#include "table.h"
#include "reftable.h"
#include "database.h"
#include "webserver/webserver.h"

#include <string.h>

void
fdb_database_init(fdb_database_t* db, fdb_mem_allocator_t* allocator)  
{
  FDB_ASSERT(((allocator == NULL) ||
                  (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
                 "Provided allocator is ill-formed.")

  if(allocator != NULL)
  {
    db->p_page_allocator = allocator; 
  }
  else
  {
    db->p_page_allocator = fdb_get_global_mem_allocator();
  }

  // Initializing allocators
  fdb_pool_alloc_init(&db->m_table_allocator, 
                      FDB_DATABASE_TABLE_ALIGNMENT, 
                      sizeof(fdb_table_t), 
                      FDB_DATABASE_TABLE_PAGE_SIZE, 
                      db->p_page_allocator);

  fdb_pool_alloc_init(&db->m_bittable_allocator, 
                        FDB_DATABASE_BITTABLE_ALIGNMENT, 
                        sizeof(fdb_bittable_t), 
                        FDB_DATABASE_BITTABLE_PAGE_SIZE, 
                        db->p_page_allocator);

  fdb_stack_alloc_init(&db->m_global_allocator, 
                       FDB_DATABASE_GLOBAL_PAGE_SIZE, 
                        db->p_page_allocator);

  // Initializing members
  fdb_webserver_init(&db->m_webserver);
  fdb_btree_init(&db->m_tags, db->p_page_allocator);
  fdb_btree_init(&db->m_tables, db->p_page_allocator);
  fdb_btree_init(&db->m_references, db->p_page_allocator);
  fdb_btree_init(&db->m_globals, db->p_page_allocator);
  fdb_mregistry_init(&db->m_mregistry, db->p_page_allocator);
  fdb_mutex_init(&db->m_mutex);
}

void
fdb_database_release(fdb_database_t* db) 
{
  fdb_database_stop_webserver(db);
  fdb_webserver_release(&db->m_webserver);
  fdb_database_clear(db);

  // releaseing members
  fdb_mutex_release(&db->m_mutex);
  fdb_mregistry_release(&db->m_mregistry);
  fdb_btree_release(&db->m_globals);
  fdb_btree_release(&db->m_references);
  fdb_btree_release(&db->m_tables);
  fdb_btree_release(&db->m_tags);

  // releaseing allocators
  fdb_stack_alloc_release(&db->m_global_allocator);
  fdb_pool_alloc_release(&db->m_bittable_allocator);
  fdb_pool_alloc_release(&db->m_table_allocator);
}

void fdb_database_clear(fdb_database_t* db) 
{
  fdb_database_lock(db);

  // cleraing tables
  fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  while (fdb_btree_iter_has_next(&it_tables)) 
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    fdb_table_t* table =  (fdb_table_t*)entry.p_value;
    fdb_table_release(table);
    fdb_pool_alloc_free(&db->m_table_allocator, table);
  }
  fdb_btree_iter_release(&it_tables);
  fdb_btree_clear(&db->m_tables);

  // clearing tag tables
  fdb_btree_iter_t it_tags;
  fdb_btree_iter_init(&it_tags, 
                      &db->m_tags);
  while (fdb_btree_iter_has_next(&it_tags))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tags);
    fdb_bittable_t* bittable = (fdb_bittable_t*)(entry.p_value);
    fdb_bittable_release(bittable);
    fdb_pool_alloc_free(&db->m_bittable_allocator, bittable);
  }
  fdb_btree_iter_release(&it_tags);
  fdb_btree_clear(&db->m_tags);

  // clearing reference tables
  fdb_btree_iter_t it_references;
  fdb_btree_iter_init(&it_references, 
                      &db->m_references);
  while (fdb_btree_iter_has_next(&it_references)) 
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_references);
    fdb_reftable_t* table =  (fdb_reftable_t*)entry.p_value;
    fdb_reftable_release(table);
    fdb_pool_alloc_free(&db->m_table_allocator, table);
  }
  fdb_btree_iter_release(&it_references);
  fdb_btree_clear(&db->m_references);

  // clearing temp tables
  fdb_btree_iter_t it_temp_tables;
  fdb_btree_iter_init(&it_temp_tables, &db->m_tables);
  while (fdb_btree_iter_has_next(&it_temp_tables)) 
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_temp_tables);
    fdb_table_t* table = (fdb_table_t*)entry.p_value;
    fdb_table_release(table);
    fdb_pool_alloc_free(&db->m_table_allocator, table);
  }
  fdb_btree_iter_release(&it_temp_tables);
  fdb_btree_clear(&db->m_tables);

  // clearing globals
  fdb_btree_iter_t it_globals;
  fdb_btree_iter_init(&it_globals, &db->m_globals);
  while (fdb_btree_iter_has_next(&it_globals)) 
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_globals);
    fdb_global_info_t* info = (fdb_global_info_t*)entry.p_value;
    if(info->m_destructor)
    {
      info->m_destructor(((fdb_global_info_t*)entry.p_value)->p_global);
    }
    fdb_stack_alloc_free(&db->m_global_allocator,((fdb_global_info_t*)entry.p_value)->p_global);
    fdb_stack_alloc_free(&db->m_global_allocator, entry.p_value);
  }
  fdb_btree_iter_release(&it_globals);
  fdb_btree_clear(&db->m_globals);

  fdb_database_unlock(db);
}

fdb_table_t*
fdb_database_create_table(fdb_database_t* db,
                          const char* name,
                          size_t csize, 
                          frs_dstr_t dstr)
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name);
  FDB_ASSERT(fdb_btree_get(&db->m_tables, hash_value) == NULL && "Table already exists");
  if(fdb_btree_get(&db->m_tables, hash_value) != NULL)
  {
    fdb_database_unlock(db);
    return NULL;
  }

  fdb_table_t* table = (fdb_table_t*)fdb_pool_alloc_alloc(&db->m_table_allocator, 
                                                          FDB_DATABASE_TABLE_ALIGNMENT, 
                                                          sizeof(fdb_table_t), 
                                                          FDB_NO_HINT);;

  fdb_table_init(table, 
                 name, 
                 hash_value, 
                 csize, 
                 dstr, 
                 db->p_page_allocator);

  fdb_btree_insert(&db->m_tables, 
               hash_value, 
               table);

  fdb_database_unlock(db);
  return table; 
}

fdb_table_t*
fdb_database_find_table(fdb_database_t* db,
                        const char* name) 
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name);
  fdb_table_t* table = (fdb_table_t*)fdb_btree_get(&db->m_tables, hash_value);
  FDB_ASSERT(table != NULL && "Find on a null table");
  fdb_database_unlock(db);
  return table;
}

fdb_table_t* 
fdb_database_find_or_create_table(fdb_database_t* db,
                                  const char* name,
                                  size_t csize, 
                                  frs_dstr_t dstr)
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name); 
  fdb_table_t* table = (fdb_table_t*)fdb_btree_get(&db->m_tables, hash_value);
  if(table != NULL) 
  {
    fdb_database_unlock(db);
    return table;
  }
  table = (fdb_table_t*)fdb_pool_alloc_alloc(&db->m_table_allocator, 
                                  FDB_DATABASE_TABLE_ALIGNMENT,
                                  sizeof(fdb_table_t), 
                                  FDB_NO_HINT);;

  fdb_table_init(table, name, 
                 hash_value, 
                 csize, 
                 dstr, 
                 db->p_page_allocator);
  fdb_btree_insert(&db->m_tables, hash_value, table);
  fdb_database_unlock(db);
  return table; 
}

void 
fdb_database_remove_table(fdb_database_t* db,
                          const char* name) 
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name);
  fdb_table_t* table = (fdb_table_t*)fdb_btree_get(&db->m_tables, hash_value);
  if(table != NULL)
  {
    fdb_table_release(table);
    fdb_pool_alloc_free(&db->m_table_allocator, table);
    fdb_btree_remove(&db->m_tables, hash_value);
  }
  fdb_database_unlock(db);
  return;
}


void 
fdb_database_clear_entity(fdb_database_t* db, 
                          entity_id_t id) 
{
  fdb_database_lock(db);
  fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  while(fdb_btree_iter_has_next(&it_tables)) 
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    fdb_table_t* table = (fdb_table_t*)entry.p_value;
    fdb_table_destroy_component(table, id);
  }
  fdb_database_unlock(db);
}

void fdb_database_tag_entity(fdb_database_t* db, 
                             entity_id_t entity_id, 
                             const char* tag) 
{
  uint32_t hash_value = hash(tag);
  fdb_database_lock(db);
  fdb_bittable_t* bittable = (fdb_bittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable == NULL) 
  {
    bittable = (fdb_bittable_t*)fdb_pool_alloc_alloc(&db->m_bittable_allocator, 
                                                     FDB_DATABASE_BITTABLE_ALIGNMENT, 
                                                     sizeof(fdb_bittable_t), 
                                                     FDB_NO_HINT);

    fdb_bittable_init(bittable, db->p_page_allocator);
    fdb_btree_insert(&db->m_tags, hash_value, bittable);
  }

  fdb_database_unlock(db);
  fdb_bittable_add(bittable, entity_id);
}

void fdb_database_untag_entity(fdb_database_t* db, 
                               entity_id_t entity_id, 
                               const char* tag) 
{
  fdb_database_lock(db);
  uint32_t hash_value = hash(tag);
  fdb_bittable_t* bittable = (fdb_bittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable != NULL) 
  {
    fdb_bittable_remove(bittable, entity_id);
  }
  fdb_database_unlock(db);
}

fdb_bittable_t* 
fdb_database_get_tagged_entities(fdb_database_t* db, 
                                 const char* tag) 
{
  uint32_t hash_value = hash(tag);
  fdb_database_lock(db);
  fdb_bittable_t* bittable = (fdb_bittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable != NULL) 
  {
    fdb_database_unlock(db);
    return bittable;
  }

  bittable = (fdb_bittable_t*)fdb_pool_alloc_alloc(&db->m_bittable_allocator, 
                                        FDB_DATABASE_BITTABLE_ALIGNMENT,  
                                        sizeof(fdb_bittable_t), 
                                        FDB_NO_HINT);

  fdb_bittable_init(bittable, db->p_page_allocator);
  fdb_btree_insert(&db->m_tags, hash_value, bittable);
  fdb_database_unlock(db);
  return bittable;
}

fdb_reftable_t*
fdb_database_find_or_create_reftable(fdb_database_t* db, 
                                     const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  fdb_database_lock(db);
  fdb_reftable_t* table_ptr = (fdb_reftable_t*)fdb_btree_get(&db->m_references, hash_value);
  if (table_ptr == NULL) 
  {

    table_ptr = (fdb_reftable_t*)fdb_pool_alloc_alloc(&db->m_table_allocator, 
                                                      FDB_DATABASE_TABLE_ALIGNMENT, 
                                                      sizeof(fdb_reftable_t), 
                                                      FDB_NO_HINT);;

    fdb_reftable_init(table_ptr, 
                      ref_name, 
                      hash_value, 
                      db->p_page_allocator);

    fdb_btree_insert(&db->m_references,  hash_value, table_ptr);
  } 
  fdb_database_unlock(db);
  return table_ptr;
}

fdb_reftable_t*
fdb_database_find_reftable(fdb_database_t* db, 
                           const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  fdb_database_lock(db);
  fdb_reftable_t* table_ptr = (fdb_reftable_t*)fdb_btree_get(&db->m_references, hash_value);
  FDB_ASSERT(table_ptr != NULL && "Cannot find ref table");
  fdb_database_unlock(db);
  return table_ptr;
}

fdb_reftable_t*
fdb_database_create_reftable(fdb_database_t* db, 
                             const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  fdb_database_lock(db);
  fdb_reftable_t* table_ptr = (fdb_reftable_t*)fdb_btree_get(&db->m_references, hash_value);
  if (table_ptr == NULL) 
  {
    table_ptr = (fdb_reftable_t*)fdb_pool_alloc_alloc(&db->m_table_allocator, 
                                                      FDB_DATABASE_TABLE_ALIGNMENT, 
                                                      sizeof(fdb_reftable_t), 
                                                      FDB_NO_HINT);;

    fdb_reftable_init(table_ptr, 
                      ref_name, 
                      hash_value, 
                      db->p_page_allocator);

    fdb_btree_insert(&db->m_references, hash_value, table_ptr);
  } 
  fdb_database_unlock(db);
  return table_ptr;
}

void
fdb_database_start_webserver(fdb_database_t* db, 
                             const char* address, 
                             const char* port)
{
  fdb_webserver_start(&db->m_webserver, 
                      db,
                      address,
                      port);
}

void
fdb_database_stop_webserver(fdb_database_t* db)
{
  fdb_webserver_stop(&db->m_webserver);
}

size_t
fdb_database_num_tables(fdb_database_t* db)
{
  fdb_database_lock(db);
  size_t size = db->m_tables.m_size;
  fdb_database_unlock(db);
  return size;
}

size_t 
fdb_database_metadata(fdb_database_t* db, 
                      fdb_table_info_t* data, 
                      uint32_t capacity)
{
  fdb_database_lock(db);
  fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  uint32_t count = 0; 
  while(fdb_btree_iter_has_next(&it_tables) && count < capacity)
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    fdb_table_t* table = (fdb_table_t*)entry.p_value;
    memcpy(&data[0], &table->m_name[0], sizeof(char)*FDB_MAX_TABLE_NAME);
    data[count].m_size = fdb_table_size(table);
    count++;
  }
  fdb_database_unlock(db);
  return count;
}

void
fdb_database_lock(fdb_database_t* db) 
{
  fdb_mutex_lock(&db->m_mutex);
}

void
fdb_database_unlock(fdb_database_t* db)
{
  fdb_mutex_unlock(&db->m_mutex);
}

uint32_t
get_table_id(const char* table_name)
{
  return hash(table_name);
}

void*
fdb_database_create_global(fdb_database_t* db,
                           const char* name,
                           size_t gsize,
                           frs_dstr_t dstr)
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name);
  fdb_global_info_t* global = (fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  FDB_ASSERT(global == NULL && "Global already exists");
  if(global != NULL)
  {
    fdb_database_unlock(db);
    return NULL;
  }
  global = (fdb_global_info_t*)fdb_stack_alloc_alloc(&db->m_global_allocator, 
                                         64, 
                                         sizeof(fdb_global_info_t), 
                                         FDB_NO_HINT);

  global->p_global = fdb_stack_alloc_alloc(&db->m_global_allocator, 
                                           64, 
                                           gsize, 
                                           FDB_NO_HINT);

  global->m_destructor = dstr;
  fdb_btree_insert(&db->m_globals, hash_value, global);
  fdb_database_unlock(db);
  return global->p_global; 
}

void 
fdb_database_remove_global(fdb_database_t* db,
                           const char* name)
{
  fdb_database_lock(db);
  uint32_t hash_value = get_table_id(name);
  fdb_global_info_t* global = (fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  if(global != NULL)
  {
    if(global->m_destructor)
    {
      global->m_destructor(global->p_global);
    }
    fdb_stack_alloc_free(&db->m_global_allocator, global->p_global); 
    fdb_stack_alloc_free(&db->m_global_allocator, global); 
    fdb_btree_remove(&db->m_globals, hash_value);
  }
  fdb_database_unlock(db);
  return;
}

void*
db_find_global(fdb_database_t* db, 
               const char* name)
{
  fdb_database_lock(db);
  void* global = fdb_database_find_global_no_lock(db, name);
  fdb_database_unlock(db);
  return global;
}

void*
fdb_database_find_global_no_lock(fdb_database_t* db, 
                                 const char* name)
{
  uint32_t hash_value = get_table_id(name);
  fdb_global_info_t* global = (fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  if(global == NULL)
  {
    return NULL;
  }
  return global->p_global;
}


void* 
fdb_database_find_or_create_global(fdb_database_t* fdb_database, 
                                   const char* name, 
                                   size_t gsize, 
                                   frs_dstr_t dstr)
{
  fdb_database_lock(fdb_database);
  uint32_t hash_value = get_table_id(name);
  fdb_global_info_t* global = (fdb_global_info_t*)fdb_btree_get(&fdb_database->m_globals, hash_value); 
  if(global == NULL)
  {
    global = (fdb_global_info_t*)fdb_stack_alloc_alloc(&fdb_database->m_global_allocator, 
                                                       64, 
                                                       sizeof(fdb_global_info_t), 
                                                       FDB_NO_HINT);

    global->p_global = fdb_stack_alloc_alloc(&fdb_database->m_global_allocator, 
                                             64, 
                                             gsize, 
                                             FDB_NO_HINT);

    global->m_destructor = dstr;
    fdb_btree_insert(&fdb_database->m_globals, hash_value, global);
  }
  fdb_database_unlock(fdb_database);
  return global->p_global; 
}

fdb_mregistry_t*
fdb_database_get_mregistry(fdb_database_t* fdb_database)
{
  return &fdb_database->m_mregistry;
}

