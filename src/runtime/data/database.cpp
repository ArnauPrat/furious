


#include "../../common/utils.h"
#include "../memory/pool_allocator.h"
#include "../memory/stack_allocator.h"
#include "bit_table.h"
#include "database.h"
#include "webserver/webserver.h"

#include <string.h>

namespace furious 
{

database_t
database_create(mem_allocator_t* allocator)  
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")

  database_t database;

  if(allocator != nullptr)
  {
    database.m_page_allocator = *allocator; 
  }
  else
  {
    database.m_page_allocator = global_mem_allocator;
  }

  // Initializing allocators
  database.m_table_allocator = pool_alloc_create(FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                                 sizeof(table_t), 
                                                 FURIOUS_DATABASE_TABLE_PAGE_SIZE, 
                                                 &database.m_page_allocator);

  database.m_bittable_allocator = pool_alloc_create(FURIOUS_DATABASE_BITTABLE_ALIGNMENT, 
                                                 sizeof(BitTable), 
                                                 FURIOUS_DATABASE_BITTABLE_PAGE_SIZE, 
                                                 &database.m_page_allocator);

  database.m_btree_allocator = pool_alloc_create(FURIOUS_BITMAP_ALIGNMENT, 
                                                 sizeof(btree_node_t), 
                                                 FURIOUS_DATABASE_BTREE_PAGE_SIZE, 
                                                 &database.m_page_allocator);

  database.m_global_allocator = stack_alloc_create(FURIOUS_DATABASE_GLOBAL_PAGE_SIZE, 
                                                   &database.m_page_allocator);

  // Initializing members
  database.m_next_entity_id = 0;
  database.p_webserver = nullptr;
  database.m_tags = btree_create(&database.m_btree_allocator);
  database.m_tables = btree_create(&database.m_btree_allocator);
  database.m_references = btree_create(&database.m_btree_allocator);
  database.m_temp_tables = btree_create(&database.m_btree_allocator);
  database.m_globals = btree_create(&database.m_btree_allocator);
  database.m_refl_data = btree_create(&database.m_btree_allocator);
  database.m_mutex = mutex_create();


  return database;
}

void
database_destroy(database_t* database) 
{
  database_stop_webserver(database);
  database_clear(database);

  btree_iter_t it_refl = btree_iter_create(&database->m_refl_data);
  while (btree_iter_has_next(&it_refl)) 
  {
    btree_entry_t entry = btree_iter_next(&it_refl);
    RefCountPtr<ReflData>* ref_data = (RefCountPtr<ReflData>*)entry.p_value;
    delete ref_data;
  }
  btree_iter_destroy(&it_refl);

  // destroying members
  mutex_destroy(&database->m_mutex);
  btree_destroy(&database->m_refl_data);
  btree_destroy(&database->m_globals);
  btree_destroy(&database->m_temp_tables);
  btree_destroy(&database->m_references);
  btree_destroy(&database->m_tables);
  btree_destroy(&database->m_tags);

  // destroying allocators
  stack_alloc_destroy(&database->m_global_allocator);
  pool_alloc_destroy(&database->m_btree_allocator);
  pool_alloc_destroy(&database->m_bittable_allocator);
  pool_alloc_destroy(&database->m_table_allocator);
}

void database_clear(database_t* database) 
{
  database_lock(database);
  btree_iter_t it_tables = btree_iter_create(&database->m_tables);
  while (btree_iter_has_next(&it_tables)) 
  {
    btree_entry_t entry = btree_iter_next(&it_tables);
    table_t* table =  (table_t*)entry.p_value;
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
  }
  btree_iter_destroy(&it_tables);
  btree_clear(&database->m_tables);

  btree_iter_t it_tags = btree_iter_create(&database->m_tags);
  while (btree_iter_has_next(&it_tags))
  {
    btree_entry_t entry = btree_iter_next(&it_tags);
    BitTable* bittable = (BitTable*)(entry.p_value);
    bittable->~BitTable();
    mem_free(&database->m_bittable_allocator, bittable);
  }
  btree_iter_destroy(&it_tags);
  btree_clear(&database->m_tags);

  btree_iter_t it_references = btree_iter_create(&database->m_references);
  while (btree_iter_has_next(&it_references)) 
  {
    btree_entry_t entry = btree_iter_next(&it_references);
    table_t* table =  (table_t*)entry.p_value;
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
  }
  btree_iter_destroy(&it_references);
  btree_clear(&database->m_references);

  btree_iter_t it_temp_tables = btree_iter_create(&database->m_temp_tables);
  while (btree_iter_has_next(&it_temp_tables)) 
  {
    btree_entry_t entry = btree_iter_next(&it_temp_tables);
    table_t* table = (table_t*)entry.p_value;
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
  }
  btree_iter_destroy(&it_temp_tables);
  btree_clear(&database->m_temp_tables);

  btree_iter_t it_globals = btree_iter_create(&database->m_globals);
  while (btree_iter_has_next(&it_globals)) 
  {
    btree_entry_t entry = btree_iter_next(&it_globals);
    ((GlobalInfo*)entry.p_value)->m_destructor(((GlobalInfo*)entry.p_value)->p_global);
    mem_free(&database->m_global_allocator,((GlobalInfo*)entry.p_value)->p_global);
    mem_free(&database->m_global_allocator, entry.p_value);
  }
  btree_iter_destroy(&it_globals);
  btree_clear(&database->m_globals);

  database_release(database);
}


entity_id_t 
database_get_next_entity_id(database_t* database) 
{
  database_lock(database);
  entity_id_t next_id = database->m_next_entity_id;
  database->m_next_entity_id++;
  database_release(database);
  return next_id;
}

void 
database_clear_entity(database_t* database, 
                      entity_id_t id) 
{
  database_lock(database);
  btree_iter_t it_tables = btree_iter_create(&database->m_tables);
  while(btree_iter_has_next(&it_tables)) 
  {
    btree_entry_t entry = btree_iter_next(&it_tables);
    table_t* table = (table_t*)entry.p_value;
    table_dealloc_component(table, id);
  }
  database_release(database);
}

void database_tag_entity(database_t* database, 
                         entity_id_t entity_id, 
                         const char* tag) 
{
  uint32_t hash_value = hash(tag);
  database_lock(database);
  BitTable* bit_table = (BitTable*)btree_get(&database->m_tags, hash_value);
  if(bit_table == nullptr) 
  {
    void* buffer = mem_alloc(&database->m_bittable_allocator, 
                             FURIOUS_DATABASE_BITTABLE_ALIGNMENT, 
                             sizeof(BitTable), 
                             FURIOUS_NO_HINT);

    bit_table = new(buffer) BitTable();
    btree_insert(&database->m_tags, hash_value, bit_table);
  }

  database_release(database);
  bit_table->add(entity_id);
}

void database_untag_entity(database_t* database, 
                           entity_id_t entity_id, 
                           const char* tag) 
{
  database_lock(database);
  uint32_t hash_value = hash(tag);
  BitTable* bit_table = (BitTable*)btree_get(&database->m_tags, hash_value);
  if(bit_table != nullptr) 
  {
    bit_table->remove(entity_id);
  }
  database_release(database);
}

BitTable* 
database_get_tagged_entities(database_t* database, 
                             const char* tag) 
{
  uint32_t hash_value = hash(tag);
  database_lock(database);
  BitTable* bit_table = (BitTable*)btree_get(&database->m_tags, hash_value);
  if(bit_table != nullptr) 
  {
    database_release(database);
    return bit_table;
  }

  void* buffer = mem_alloc(&database->m_bittable_allocator, 
                           FURIOUS_DATABASE_BITTABLE_ALIGNMENT,  
                           sizeof(BitTable), 
                           FURIOUS_NO_HINT);

  bit_table = new(buffer) BitTable();
  btree_insert(&database->m_tags, hash_value, bit_table);
  database_release(database);
  return bit_table;
}

TableView<uint32_t>
database_find_or_create_ref_table(database_t* database, 
                                  const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  database_lock(database);
  table_t* table_ptr = (table_t*)btree_get(&database->m_references, hash_value);
  if (table_ptr == nullptr) 
  {
    
    table_ptr = (table_t*)mem_alloc(&database->m_table_allocator, 
                                    FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                    sizeof(table_t), 
                                    FURIOUS_NO_HINT);;

    *table_ptr = table_create(ref_name, 
                              hash_value, 
                              sizeof(uint32_t), 
                              destructor<uint32_t>);

    btree_insert(&database->m_references,  hash_value, table_ptr);
  } 
  database_release(database);
  return TableView<uint32_t>(table_ptr);
}

TableView<uint32_t>
database_find_ref_table(database_t* database, 
                        const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  database_lock(database);
  table_t* table_ptr = (table_t*)btree_get(&database->m_references, hash_value);
  FURIOUS_ASSERT(table_ptr != nullptr && "Cannot find ref table");
  database_release(database);
  return TableView<uint32_t>(table_ptr);
}

TableView<uint32_t>
database_create_ref_table(database_t* database, 
                          const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  database_lock(database);
  table_t* table_ptr = (table_t*)btree_get(&database->m_references, hash_value);
  if (table_ptr == nullptr) 
  {
    table_ptr = (table_t*)mem_alloc(&database->m_table_allocator, 
                                    FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                    sizeof(table_t), 
                                    FURIOUS_NO_HINT);;

    *table_ptr = table_create(ref_name, 
                              hash_value, 
                              sizeof(uint32_t), 
                              destructor<uint32_t>);
    
    btree_insert(&database->m_references, hash_value, table_ptr);
  } 
  database_release(database);
  return TableView<uint32_t>(table_ptr);
}

void 
database_add_reference(database_t* database, 
                       const char* ref_name, 
                       entity_id_t tail, 
                       entity_id_t head) 
{
  uint32_t hash_value = hash(ref_name);
  database_lock(database);
  table_t* table_ptr = (table_t*)btree_get(&database->m_references, hash_value);
  if (table_ptr == nullptr) 
  {
    table_ptr = (table_t*)mem_alloc(&database->m_table_allocator, 
                                    FURIOUS_DATABASE_TABLE_ALIGNMENT, 
                                    sizeof(table_t), 
                                    FURIOUS_NO_HINT);;

    *table_ptr = table_create(ref_name, 
                              hash_value, 
                              sizeof(uint32_t),
                              destructor<uint32_t>);

    btree_insert(&database->m_references, hash_value, table_ptr);
  } 

  TableView<uint32_t> t_view(table_ptr);
  t_view.insert_component(tail, head);
  database_release(database);
  return;
}

void 
database_remove_reference(database_t* database, 
                          const char* ref_name, 
                          entity_id_t tail) 
{
  uint32_t hash_value = hash(ref_name);
  database_lock(database);
  table_t* table_ptr = (table_t*)btree_get(&database->m_references, hash_value);
  if (table_ptr != nullptr) 
  {
    table_dealloc_component(table_ptr, tail);
  }
  database_release(database);
  return;
}

void
database_start_webserver(database_t* database, 
                         const char* address, 
                         const char* port)
{
  database->p_webserver = new WebServer();
  database->p_webserver->start(database,
                               address,
                               port);
}

void
database_stop_webserver(database_t* database)
{
  if(database->p_webserver != nullptr)
  {
    database->p_webserver->stop();
    delete database->p_webserver;
    database->p_webserver = nullptr;
  }
}

size_t
database_num_tables(database_t* database)
{
  database_lock(database);
  size_t size = database->m_tables.m_size;
  database_release(database);
  return size;
}

size_t 
database_metadata(database_t* database, 
                   TableInfo* data, 
                   uint32_t capacity)
{
  database_lock(database);
  btree_iter_t it_tables = btree_iter_create(&database->m_tables);
  uint32_t count = 0; 
  while(btree_iter_has_next(&it_tables) && count < capacity)
  {
    btree_entry_t entry = btree_iter_next(&it_tables);
    table_t* table = (table_t*)entry.p_value;
    memcpy(&data[0], &table->m_name[0], sizeof(char)*FURIOUS_MAX_TABLE_NAME);
    data[count].m_size = table_size(table);
    count++;
  }
  database_release(database);
  return count;
}

void
database_lock(database_t* database) 
{
  mutex_lock(&database->m_mutex);
}

void
database_release(database_t* database)
{
  mutex_unlock(&database->m_mutex);
}

uint32_t
get_table_id(const char* table_name)
{
  return hash(table_name);
}

void
database_remove_temp_tables(database_t* database)
{
  database_lock(database);
  database_remove_temp_tables_no_lock(database);
  database_release(database);
}

void
database_remove_temp_tables_no_lock(database_t* database)
{
  btree_iter_t it_temp_tables = btree_iter_create(&database->m_temp_tables);
  while (btree_iter_has_next(&it_temp_tables)) 
  {
    btree_entry_t entry = btree_iter_next(&it_temp_tables);
    table_t* table = (table_t*)entry.p_value;
    table_destroy(table);
    mem_free(&database->m_table_allocator, table);
  }
  btree_iter_destroy(&it_temp_tables);
  btree_clear(&database->m_temp_tables);
}


} /* furious */ 
