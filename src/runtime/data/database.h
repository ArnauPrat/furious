

#ifndef _FDB_DATABASE_H_
#define _FDB_DATABASE_H_

#include "macros.h"
#include "../../common/platform.h"
#include "../../common/btree.h"
#include "../../common/mutex.h"
#include "../../common/memory/stack_allocator.h"
#include "../../common/memory/pool_allocator.h"
#include "txtable.h"
#include "txbittable.h"
#include "tx/txheap_allocator.h"
#include "reflection.h"
#include "webserver/webserver.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

struct fdb_mdata_t;

struct fdb_table_info_t 
{
  char        m_name[FDB_MAX_TABLE_NAME];
  uint32_t    m_size;
};

typedef void (*frs_dstr_t)(void*) ;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct fdb_global_info_t 
{
  struct fdb_txheap_alloc_ref_t m_ref;
  void (*m_destructor)(void *ptr);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * \brief Stores all the tables and globals
 */
struct fdb_database_t 
{
  struct fdb_btree_t                   m_tags;                  //< btree with the bittable holding the entities with tags 
  struct fdb_btree_t                   m_tables;                //< btree with the component tables 
  struct fdb_btree_t                   m_references;            //< btree with the reference tables 
  struct fdb_btree_t                   m_globals;               //< btree with the globals 
  struct fdb_webserver_t               m_webserver;             //< The db web server
  struct fdb_mregistry_t               m_mregistry;             //< The metadata registry

  struct fdb_mem_allocator_t*          p_page_allocator;        //< Allocator used to allocate pages for the rest of allocators
  struct fdb_pool_alloc_t              m_txtable_allocator;     //< Allocator for tables, temporal tables and reference tables
  struct fdb_txtable_factory_t         m_txtable_factory;       //< Factory for txtables
  struct fdb_pool_alloc_t              m_txbittable_allocator;  //< Allocator for bittables
  struct fdb_txbittable_factory_t      m_txbittable_factory;    //< Factory for txtables
  struct fdb_btree_factory_t           m_btree_factory;
  struct fdb_pool_alloc_t              m_global_info_allocator;      //< Allocator for globals
  struct fdb_txheap_alloc_t            m_global_allocator; 
};


/**
 * \brief Creates a furious db
 *
 * \return The created db
 */
void
fdb_database_init(struct fdb_database_t* db, 
                  struct fdb_mem_allocator_t* allocator);

/**
 * \brief Destroys a db
 *
 * \param db The db to destroy
 */
void
fdb_database_release(struct fdb_database_t* db, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Creates a table in the db
 *
 * \param db The db to create the table into
 * \param name The name of the table
 * \param csize The component size of the table
 * \param dstr The destructor for the components
 *
 * \return Returns a pointer to the table. nullptr if the table already exists 
 */
struct fdb_txtable_t*
fdb_database_create_table(struct fdb_database_t* db,
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          const char* name,
                          size_t csize, 
                          frs_dstr_t dstr);


/**
 * \brief Finds a table in the db
 *
 * \param db The db to find the table from
 * \param name The name of the db to get
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns a pointer to the table. nullptr if the table cannot be found
 */
struct fdb_txtable_t* 
fdb_database_find_table(struct fdb_database_t* db, 
                        const char* name);

/**
 * \brief Finds the table for the given name or creates if it does not exist
 *
 * \param db The db to find the table from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the table
 * \param csize The component size of the table
 * \param dstr The destructor for the components
 *
 * \return A pointer to the created table 
 */
struct fdb_txtable_t*
fdb_database_find_or_create_table(struct fdb_database_t* db,
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx,
                                  const char* name,
                                  size_t csize, 
                                  frs_dstr_t dstr);
/**
 * \brief Removes a table
 *
 * \param db The db to find the table from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 */
void
fdb_database_remove_table(struct fdb_database_t* db, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          const char* name);

/**
 * \brief Clears and removes all the tables from the db
 *
 * \param db The db to find the table from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 */
void 
fdb_database_clear(struct fdb_database_t* db, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Clears all the rows of a given element 
 *
 * \param db The db to find the table from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param id The id of the element to remove
 *
 */
void
fdb_database_clear_entity(struct fdb_database_t* db, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t id);

/**
 * \brief Tags an entity with the given tag
 *
 * \param entity_id The id of the entity to tag
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param tag The tag to tag the entity with
 */
void 
fdb_database_tag_entity(struct fdb_database_t* db, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx,
                        entity_id_t entity_id, 
                        const char* tag);

/**
 * \brief Untags an entity from a given tag
 *
 * \param entity_id The entity to untag
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param tag The tag to remove from the entity
 */
void 
fdb_database_untag_entity(struct fdb_database_t* db, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t entity_id, 
                          const char* tag);

/**
 * \brief Gets a bitset with the entities with a given tag
 *
 * \param tag The tag to get the entities for
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * 
 * \return Returns a bitset with the entities of the tag
 */
struct fdb_txbittable_t*
fdb_database_get_tagged_entities(struct fdb_database_t* db, 
                                 struct fdb_tx_t* tx, 
                                 struct fdb_txthread_ctx_t* txtctx,
                                 const char* tag);

/**
 * \brief Finds or Creates a reference table
 *
 * \param ref_name The reference name
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns a TableView of the reference table
 */
struct fdb_txtable_t*
fdb_database_find_or_create_reftable(struct fdb_database_t* db, 
                                     struct fdb_tx_t* tx, 
                                     struct fdb_txthread_ctx_t* txtctx,
                                     const char* ref_name);

/**
 * \brief  Finds a ref  table
 *
 * \param ref_name The reference table to find
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns a table view of the reference table
 */
struct fdb_txtable_t*
fdb_database_find_reftable(struct fdb_database_t* db, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           const char* ref_name);

/**
 * \brief Creates a ref  table
 *
 * \param ref_name The reference table to find
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns a table view of the reference table
 */
struct fdb_txtable_t*
fdb_database_create_reftable(struct fdb_database_t* db, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx,
                             const char* ref_name);


/**
 * \brief Starts a webserver to listen to the given address and port
 */
void
fdb_database_start_webserver(struct fdb_database_t* db, 
                             const char* address, 
                             const char* port);

/**
 * \brief Stops the web server
 */
void
fdb_database_stop_webserver(struct fdb_database_t* db);

/**
 * \brief Gets the table metadata of the different tables in the db  
 *
 * \param data A pointer to TableInfo array 
 * \param capacity The capacity of the TableInfo array
 *
 * \return Returns the number of entries set in the array
 */
size_t 
fdb_database_metadata(struct fdb_database_t* db, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      struct fdb_table_info_t* data, 
                      uint32_t capacity);

/**
 * \brief Returns the number of tables in the db
 *
 * \return 
 */
size_t
fdb_database_num_tables(struct fdb_database_t* db);

/**
 * \brief Creates a global
 *
 * \param db The db to create the global to
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the global
 * \param gsize The size in bytes of the global
 *
 * \return Returns a pointer to the created global. Returns nullptr if it 
 * cannot be created
 */
void*
fdb_database_create_global(struct fdb_database_t* db, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           const char* name,
                           size_t gsize,
                           frs_dstr_t dstr);


/**
 * \brief Removes an existing global
 *
 * \param db The db to remove the global from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the global to remove
 *
 * \tparam T The global to remove 
 */
void 
fdb_database_remove_global(struct fdb_database_t* db, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           const char* name);

/**
 * \brief Gets the global of a given type
 *
 * \param db The db to find the global into
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the global 
 * \param write Whether we want to write to this global or not
 *
 * \return A pointer to the global. Returns nullptr if the global could not be
 * found
 */
void*
fdb_database_find_global(struct fdb_database_t* db, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx,
                         const char* name, 
                         bool write);

/**
 * \brief Gets the global of a given type. Non-locking version meant to be
 * used by the code generated by the compiler
 *
 * \param db The db to find the global into
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the global 
 * \param write Whether we want to write to this global or not
 *
 * \return A pointer to the global. Returns nullptr if the global could not be
 * found
 */
void*
fdb_database_find_global_no_lock(struct fdb_database_t* db, 
                                 struct fdb_tx_t* tx, 
                                 struct fdb_txthread_ctx_t* txtctx,
                                 const char* name, 
                                 bool write);

/**
 * \brief Finds or creates a new global
 *
 * \param db The db to create or find the global into 
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param name The name of the global
 * \param gsize The size in bytes of the global
 * \param dstr The destructor to call when removing the global
 * \param write Whether we want to write to this global or not
 *
 * \return Returns a pointer to the global.
 */
void*
fdb_database_find_or_create_global(struct fdb_database_t* db,
                                   struct fdb_tx_t* tx, 
                                   struct fdb_txthread_ctx_t* txtctx,
                                   const char* name, 
                                   size_t gsize, 
                                   frs_dstr_t dstr, 
                                   bool write);


/**
 * \brief Gets the metadata registry of the db
 *
 * \param db The db to get the registry from
 *
 * \return Returns the metadata registry
 */
struct fdb_mregistry_t*
fdb_database_get_mregistry(struct fdb_database_t* db);


uint32_t
get_table_id(const char* table_name);

#ifdef __cplusplus
}
#endif


#endif
