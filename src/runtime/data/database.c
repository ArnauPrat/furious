
#include "../../common/utils.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/memory/stack_allocator.h"
#include "txbittable.h"
#include "txtable.h"
#include "database.h"
#include "webserver/webserver.h"

#include <string.h>

void
fdb_database_init(struct fdb_database_t* db, struct fdb_mem_allocator_t* allocator)  
{
  FDB_ASSERT(((allocator == NULL) ||
                  (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
                 "Provided allocator is ill-formed.")

  db->p_page_allocator = allocator; 

  // Initializing allocators
  fdb_pool_alloc_init(&db->m_txtable_allocator, 
                      FDB_DATABASE_TABLE_ALIGNMENT, 
                      sizeof(struct fdb_txtable_t), 
                      FDB_DATABASE_TABLE_PAGE_SIZE, 
                      db->p_page_allocator);

  fdb_txtable_factory_init(&db->m_txtable_factory, 
                           db->p_page_allocator);

  fdb_pool_alloc_init(&db->m_txbittable_allocator, 
                        FDB_DATABASE_BITTABLE_ALIGNMENT, 
                        sizeof(struct fdb_txbittable_t), 
                        FDB_DATABASE_BITTABLE_PAGE_SIZE, 
                        db->p_page_allocator);

  fdb_txbittable_factory_init(&db->m_txbittable_factory, 
                              db->p_page_allocator);

  fdb_btree_factory_init(&db->m_btree_factory, 
                         db->p_page_allocator);

  fdb_pool_alloc_init(&db->m_global_info_allocator, 
                      FDB_DATABASE_GLOBAL_INFO_ALIGNMENT, 
                      sizeof(struct fdb_global_info_t),
                      FDB_DATABASE_GLOBAL_INFO_PAGE_SIZE,
                      db->p_page_allocator);

  fdb_txheap_alloc_init(&db->m_global_allocator, 
                        FDB_DATABASE_GLOBAL_ALIGNMENT, 
                        FDB_DATABASE_GLOBAL_PAGE_SIZE, 
                        db->p_page_allocator);

  // Initializing members
#ifdef FDB_ENABLE_WEBSERVER
  fdb_webserver_init(&db->m_webserver);
#endif
  fdb_btree_init(&db->m_tags, 
                 &db->m_btree_factory);
  fdb_btree_init(&db->m_tables,
                 &db->m_btree_factory);
  fdb_btree_init(&db->m_references, 
                 &db->m_btree_factory);
  fdb_btree_init(&db->m_globals,
                 &db->m_btree_factory);
  fdb_mregistry_init(&db->m_mregistry, db->p_page_allocator);
}

void
fdb_database_release(struct fdb_database_t* db) 
{
#ifdef FDB_ENABLE_WEBSERVER
  fdb_database_stop_webserver(db);
  fdb_webserver_release(&db->m_webserver);
#endif
  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_database_clear(db, 
                     &tx, 
                     txtctx);

  // releaseing members
  fdb_mregistry_release(&db->m_mregistry);
  fdb_btree_release(&db->m_globals);
  fdb_btree_release(&db->m_references);
  fdb_btree_release(&db->m_tables);
  fdb_btree_release(&db->m_tags);

  // releaseing allocators
  fdb_txheap_alloc_release(&db->m_global_allocator, 
                           &tx, 
                           txtctx);
  fdb_pool_alloc_release(&db->m_global_info_allocator);
  fdb_btree_factory_release(&db->m_btree_factory);
  fdb_txbittable_factory_release(&db->m_txbittable_factory, 
                                 &tx, 
                                 txtctx);
  fdb_pool_alloc_release(&db->m_txbittable_allocator);
  fdb_txtable_factory_release(&db->m_txtable_factory, 
                              &tx, 
                              txtctx);
  fdb_pool_alloc_release(&db->m_txtable_allocator);
  fdb_tx_commit(&tx);
}

void fdb_database_clear(struct fdb_database_t* db,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx) 
{
  // cleraing tables
  struct fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  while (fdb_btree_iter_has_next(&it_tables)) 
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    struct fdb_txtable_t* table =  (struct fdb_txtable_t*)entry.p_value;
    fdb_txtable_release(table, 
                        tx, 
                        txtctx);
    fdb_pool_alloc_free(&db->m_txtable_allocator, table);
  }
  fdb_btree_iter_release(&it_tables);
  fdb_btree_clear(&db->m_tables);

  /*
  // clearing tag tables
  struct fdb_btree_iter_t it_tags;
  fdb_btree_iter_init(&it_tags, 
                      &db->m_tags);
  while (fdb_btree_iter_has_next(&it_tags))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tags);
    struct fdb_txbittable_t* bittable = (struct fdb_txbittable_t*)(entry.p_value);
    fdb_txbittable_release(bittable, 
                           tx, 
                           txtctx);
    fdb_pool_alloc_free(&db->m_txbittable_allocator, bittable);
  }
  fdb_btree_iter_release(&it_tags);
  fdb_btree_clear(&db->m_tags);

  // clearing reference tables
  struct fdb_btree_iter_t it_references;
  fdb_btree_iter_init(&it_references, 
                      &db->m_references);
  while (fdb_btree_iter_has_next(&it_references)) 
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_references);
    struct fdb_txtable_t* table =  (struct fdb_txtable_t*)entry.p_value;
    fdb_txtable_release(table, 
                        tx, 
                        txtctx);
    fdb_pool_alloc_free(&db->m_txtable_allocator, table);
  }
  fdb_btree_iter_release(&it_references);
  fdb_btree_clear(&db->m_references);

  // clearing globals
  struct fdb_btree_iter_t it_globals;
  fdb_btree_iter_init(&it_globals, &db->m_globals);
  while (fdb_btree_iter_has_next(&it_globals)) 
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_globals);
    struct fdb_global_info_t* info = (struct fdb_global_info_t*)entry.p_value;
    if(info->m_destructor)
    {
      void* global_ptr = fdb_txheap_alloc_ptr(&db->m_global_allocator, 
                                              tx, 
                                              txtctx, 
                                              ((struct fdb_global_info_t*)entry.p_value)->m_ref, 
                                              true);
      info->m_destructor(global_ptr);
    }
    fdb_txheap_alloc_free(&db->m_global_allocator,
                          tx, 
                          txtctx, 
                          ((struct fdb_global_info_t*)entry.p_value)->m_ref);
    fdb_pool_alloc_free(&db->m_global_info_allocator, entry.p_value);
  }
  fdb_btree_iter_release(&it_globals);
  fdb_btree_clear(&db->m_globals);
  */

}

struct fdb_txtable_t*
fdb_database_create_table(struct fdb_database_t* db,
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          const char* name,
                          size_t csize, 
                          frs_dstr_t dstr)
{
  uint32_t hash_value = get_table_id(name);
  FDB_ASSERT(fdb_btree_get(&db->m_tables, hash_value) == NULL && "Table already exists");
  if(fdb_btree_get(&db->m_tables, hash_value) != NULL)
  {
    return NULL;
  }

  struct fdb_txtable_t* table = (struct fdb_txtable_t*)fdb_pool_alloc_alloc(&db->m_txtable_allocator, 
                                                                            FDB_DATABASE_TABLE_ALIGNMENT, 
                                                                            sizeof(struct fdb_txtable_t), 
                                                                            FDB_NO_HINT);;

  fdb_txtable_init(table, 
                   &db->m_txtable_factory,
                   tx, 
                   txtctx, 
                   name, 
                   hash_value, 
                   csize, 
                   dstr, 
                   db->p_page_allocator);

  fdb_btree_insert(&db->m_tables, 
               hash_value, 
               table);

  return table; 
}

struct fdb_txtable_t*
fdb_database_find_table(struct fdb_database_t* db,
                        const char* name) 
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_txtable_t* table = (struct fdb_txtable_t*)fdb_btree_get(&db->m_tables, hash_value);
  FDB_ASSERT(table != NULL && "Find on a null table");
  return table;
}

struct fdb_txtable_t* 
fdb_database_find_or_create_table(struct fdb_database_t* db,
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx,
                                  const char* name,
                                  size_t csize, 
                                  frs_dstr_t dstr)
{
  uint32_t hash_value = get_table_id(name); 
  struct fdb_txtable_t* table = (struct fdb_txtable_t*)fdb_btree_get(&db->m_tables, hash_value);
  if(table != NULL) 
  {
    return table;
  }
  table = (struct fdb_txtable_t*)fdb_pool_alloc_alloc(&db->m_txtable_allocator, 
                                                      FDB_DATABASE_TABLE_ALIGNMENT,
                                                      sizeof(struct fdb_txtable_t), 
                                                      FDB_NO_HINT);;

  fdb_txtable_init(table, 
                   &db->m_txtable_factory,
                   tx, 
                   txtctx,
                   name, 
                   hash_value, 
                   csize, 
                   dstr, 
                   db->p_page_allocator);

  fdb_btree_insert(&db->m_tables, hash_value, table);
  return table; 
}

void 
fdb_database_remove_table(struct fdb_database_t* db,
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          const char* name) 
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_txtable_t* table = (struct fdb_txtable_t*)fdb_btree_get(&db->m_tables, hash_value);
  if(table != NULL)
  {
    fdb_txtable_release(table, 
                        tx, 
                        txtctx);
    fdb_pool_alloc_free(&db->m_txtable_allocator, table);
    fdb_btree_remove(&db->m_tables, hash_value);
  }
  return;
}


void 
fdb_database_clear_entity(struct fdb_database_t* db, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t id) 
{
  struct fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  while(fdb_btree_iter_has_next(&it_tables)) 
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    struct fdb_txtable_t* table = (struct fdb_txtable_t*)entry.p_value;
    fdb_txtable_destroy_component(table,
                                  tx, 
                                  txtctx, 
                                  id);
  }
}

void fdb_database_tag_entity(struct fdb_database_t* db, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx,
                             entity_id_t entity_id, 
                             const char* tag) 
{
  uint32_t hash_value = hash(tag);
  struct fdb_txbittable_t* bittable = (struct fdb_txbittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable == NULL) 
  {
    bittable = (struct fdb_txbittable_t*)fdb_pool_alloc_alloc(&db->m_txbittable_allocator, 
                                                              FDB_DATABASE_BITTABLE_ALIGNMENT, 
                                                              sizeof(struct fdb_txbittable_t), 
                                                              FDB_NO_HINT);

    fdb_txbittable_init(bittable, 
                        &db->m_txbittable_factory, 
                        tx, 
                        txtctx);
    fdb_btree_insert(&db->m_tags, hash_value, bittable);
  }

  fdb_txbittable_add(bittable, 
                     tx, 
                     txtctx, 
                     entity_id);
}

void fdb_database_untag_entity(struct fdb_database_t* db, 
                               struct fdb_tx_t* tx, 
                               struct fdb_txthread_ctx_t* txtctx,
                               entity_id_t entity_id, 
                               const char* tag) 
{
  uint32_t hash_value = hash(tag);
  struct fdb_txbittable_t* bittable = (struct fdb_txbittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable != NULL) 
  {
    fdb_txbittable_remove(bittable, 
                          tx, 
                          txtctx, 
                          entity_id);
  }
}

struct fdb_txbittable_t* 
fdb_database_get_tagged_entities(struct fdb_database_t* db, 
                                 struct fdb_tx_t* tx, 
                                 struct fdb_txthread_ctx_t* txtctx,
                                 const char* tag) 
{
  uint32_t hash_value = hash(tag);
  struct fdb_txbittable_t* bittable = (struct fdb_txbittable_t*)fdb_btree_get(&db->m_tags, hash_value);
  if(bittable != NULL) 
  {
    return bittable;
  }

  bittable = (struct fdb_txbittable_t*)fdb_pool_alloc_alloc(&db->m_txbittable_allocator, 
                                        FDB_DATABASE_BITTABLE_ALIGNMENT,  
                                        sizeof(struct fdb_txbittable_t), 
                                        FDB_NO_HINT);

  fdb_txbittable_init(bittable, 
                      &db->m_txbittable_factory, 
                      tx, 
                      txtctx);
  fdb_btree_insert(&db->m_tags, hash_value, bittable);
  return bittable;
}

struct fdb_txtable_t*
fdb_database_find_or_create_reftable(struct fdb_database_t* db, 
                                     struct fdb_tx_t* tx, 
                                     struct fdb_txthread_ctx_t* txtctx,
                                     const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  struct fdb_txtable_t* table_ptr = (struct fdb_txtable_t*)fdb_btree_get(&db->m_references, hash_value);
  if (table_ptr == NULL) 
  {

    table_ptr = (struct fdb_txtable_t*)fdb_pool_alloc_alloc(&db->m_txtable_allocator, 
                                                            FDB_DATABASE_TABLE_ALIGNMENT, 
                                                            sizeof(struct fdb_txtable_t), 
                                                            FDB_NO_HINT);;

    fdb_txtable_init(table_ptr, 
                     &db->m_txtable_factory, 
                     tx, 
                     txtctx, 
                     ref_name, 
                     hash_value, 
                     sizeof(entity_id_t),
                     NULL,
                     db->p_page_allocator);

    fdb_btree_insert(&db->m_references,  hash_value, table_ptr);
  } 
  return table_ptr;
}

struct fdb_txtable_t*
fdb_database_find_reftable(struct fdb_database_t* db, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx, 
                           const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  struct fdb_txtable_t* table_ptr = (struct fdb_txtable_t*)fdb_btree_get(&db->m_references, hash_value);
  FDB_ASSERT(table_ptr != NULL && "Cannot find ref table");
  return table_ptr;
}

struct fdb_txtable_t*
fdb_database_create_reftable(struct fdb_database_t* db, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             const char* ref_name)
{
  uint32_t hash_value = hash(ref_name);
  struct fdb_txtable_t* table_ptr = (struct fdb_txtable_t*)fdb_btree_get(&db->m_references, hash_value);
  if (table_ptr == NULL) 
  {
    table_ptr = (struct fdb_txtable_t*)fdb_pool_alloc_alloc(&db->m_txtable_allocator, 
                                                            FDB_DATABASE_TABLE_ALIGNMENT, 
                                                            sizeof(struct fdb_txtable_t), 
                                                            FDB_NO_HINT);;

    fdb_txtable_init(table_ptr, 
                     &db->m_txtable_factory, 
                     tx, 
                     txtctx,
                     ref_name, 
                     hash_value, 
                     sizeof(entity_id_t),
                     NULL,
                     db->p_page_allocator);

    fdb_btree_insert(&db->m_references, hash_value, table_ptr);
  } 
  return table_ptr;
}

void
fdb_database_start_webserver(struct fdb_database_t* db, 
                             const char* address, 
                             const char* port)
{
  
#ifdef FDB_ENABLE_WEBSERVER
  fdb_webserver_start(&db->m_webserver, 
                      db,
                      address,
                      port);
#endif
}

void
fdb_database_stop_webserver(struct fdb_database_t* db)
{
#ifdef FDB_ENABLE_WEBSERVER
  fdb_webserver_stop(&db->m_webserver);
#endif
}

size_t
fdb_database_num_tables(struct fdb_database_t* db)
{
  size_t size = db->m_tables.m_size;
  return size;
}

size_t 
fdb_database_metadata(struct fdb_database_t* db, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      struct fdb_table_info_t* data, 
                      uint32_t capacity)
{
  struct fdb_btree_iter_t it_tables;
  fdb_btree_iter_init(&it_tables, &db->m_tables);
  uint32_t count = 0; 
  while(fdb_btree_iter_has_next(&it_tables) && count < capacity)
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&it_tables);
    struct fdb_txtable_t* table = (struct fdb_txtable_t*)entry.p_value;
    memcpy(&data[0], &table->m_name[0], sizeof(char)*FDB_MAX_TABLE_NAME);
    data[count].m_size = fdb_txtable_size(table, 
                                          tx, 
                                          txtctx);
    count++;
  }
  return count;
}

uint32_t
get_table_id(const char* table_name)
{
  return hash(table_name);
}

void*
fdb_database_create_global(struct fdb_database_t* db,
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx, 
                           const char* name,
                           size_t gsize,
                           frs_dstr_t dstr)
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_global_info_t* global = (struct fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  FDB_ASSERT(global == NULL && "Global already exists");
  if(global != NULL)
  {
    return NULL;
  }
  global = (struct fdb_global_info_t*)fdb_pool_alloc_alloc(&db->m_global_info_allocator, 
                                                           64, 
                                                           sizeof(struct fdb_global_info_t), 
                                                           FDB_NO_HINT);

  global->m_ref = fdb_txheap_alloc_alloc(&db->m_global_allocator, 
                                         tx, 
                                         txtctx,
                                         FDB_DATABASE_GLOBAL_ALIGNMENT, 
                                         gsize, 
                                         FDB_NO_HINT);

  void* ptr = fdb_txheap_alloc_ptr(&db->m_global_allocator, 
                                     tx, 
                                     txtctx, 
                                     global->m_ref, 
                                     true);

  global->m_destructor = dstr;
  fdb_btree_insert(&db->m_globals, hash_value, global);
  return ptr; 
}

void 
fdb_database_remove_global(struct fdb_database_t* db,
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           const char* name)
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_global_info_t* global = (struct fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  if(global != NULL)
  {
    if(global->m_destructor)
    {
      void* ptr = fdb_txheap_alloc_ptr(&db->m_global_allocator, 
                                       tx, 
                                       txtctx, 
                                       global->m_ref, 
                                       true);
      global->m_destructor(ptr);
    }
    fdb_txheap_alloc_free(&db->m_global_allocator, 
                          tx, 
                          txtctx, 
                          global->m_ref); 
    fdb_pool_alloc_free(&db->m_global_info_allocator, global); 
    fdb_btree_remove(&db->m_globals, hash_value);
  }
  return;
}

void*
fdb_database_find_global(struct fdb_database_t* db, 
                struct fdb_tx_t* tx, 
                struct fdb_txthread_ctx_t* txtctx,
                const char* name, 
                bool write)
{
  void* global = fdb_database_find_global_no_lock(db, 
                                                  tx, 
                                                  txtctx, 
                                                  name, 
                                                  write);
  return global;
}

void*
fdb_database_find_global_no_lock(struct fdb_database_t* db, 
                                 struct fdb_tx_t* tx, 
                                 struct fdb_txthread_ctx_t* txtctx,
                                 const char* name, 
                                 bool write)
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_global_info_t* global = (struct fdb_global_info_t*)fdb_btree_get(&db->m_globals, hash_value);
  if(global == NULL)
  {
    return NULL;
  }
  void* ptr = fdb_txheap_alloc_ptr(&db->m_global_allocator, 
                                   tx, 
                                   txtctx, 
                                   global->m_ref, 
                                   write);
  return ptr;
}


void* 
fdb_database_find_or_create_global(struct fdb_database_t* fdb_database, 
                                   struct fdb_tx_t* tx, 
                                   struct fdb_txthread_ctx_t* txtctx,
                                   const char* name, 
                                   size_t gsize, 
                                   frs_dstr_t dstr, 
                                   bool write)
{
  uint32_t hash_value = get_table_id(name);
  struct fdb_global_info_t* global = (struct fdb_global_info_t*)fdb_btree_get(&fdb_database->m_globals, hash_value); 
  if(global == NULL)
  {
    global = (struct fdb_global_info_t*)fdb_pool_alloc_alloc(&fdb_database->m_global_info_allocator, 
                                                              64, 
                                                              sizeof(struct fdb_global_info_t), 
                                                              FDB_NO_HINT);

    global->m_ref = fdb_txheap_alloc_alloc(&fdb_database->m_global_allocator, 
                                           tx, 
                                           txtctx, 
                                           64, 
                                           gsize, 
                                           FDB_NO_HINT);

    global->m_destructor = dstr;
    fdb_btree_insert(&fdb_database->m_globals, hash_value, global);
  }
  void* ptr = fdb_txheap_alloc_ptr(&fdb_database->m_global_allocator, 
                                   tx, 
                                   txtctx, 
                                   global->m_ref, 
                                   write);

  return ptr; 
}

struct fdb_mregistry_t*
fdb_database_get_mregistry(struct fdb_database_t* fdb_database)
{
  return &fdb_database->m_mregistry;
}

