

#ifndef _FDB_DATABASE_H_
#define _FDB_DATABASE_H_

#include "macros.h"
#include "../../common/platform.h"
#include "../../common/btree.h"
#include "../../common/mutex.h"
#include "../../common/memory/stack_allocator.h"
#include "../../common/memory/pool_allocator.h"
#include "reflection.h"
#include "webserver/webserver.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_bittable_t fdb_bittable_t;
typedef struct fdb_table_t fdb_table_t;
typedef struct fdb_reftable_t fdb_reftable_t;
typedef struct fdb_mdata_t fdb_mdata_t;

typedef struct fdb_table_info_t 
{
  char        m_name[FDB_MAX_TABLE_NAME];
  uint32_t    m_size;
} fdb_table_info_t;

typedef void (*frs_dstr_t)(void*) ;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef struct fdb_global_info_t 
{
  FDB_ALIGNED(void*, p_global, 64);
  void (*m_destructor)(void *ptr);
} fdb_global_info_t;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * \brief Stores all the tables and globals
 */
typedef struct fdb_database_t 
{
  fdb_btree_t                   m_tags;               //< btree with the bittable holding the entities with tags 
  fdb_btree_t                   m_tables;             //< btree with the component tables 
  fdb_btree_t                   m_references;         //< btree with the reference tables 
  fdb_btree_t                   m_globals;            //< btree with the globals 
  fdb_webserver_t               m_webserver;          //< The db web server
  fdb_mregistry_t               m_mregistry;          //< The metadata registry
  fdb_mutex_t                   m_mutex;              //< The mutex to exclusive access to the db

  fdb_mem_allocator_t*          p_page_allocator;     //< Allocator used to allocate pages for the rest of allocators
  fdb_pool_alloc_t              m_table_allocator;    //< Allocator for tables, temporal tables and reference tables
  fdb_pool_alloc_t              m_bittable_allocator; //< Allocator for bittables
  fdb_stack_alloc_t             m_global_allocator;   //< Allocator for globals
} fdb_database_t;


/**
 * \brief Creates a furious db
 *
 * \return The created db
 */
void
fdb_database_init(fdb_database_t* db, 
                  fdb_mem_allocator_t* allocator);

/**
 * \brief Destroys a db
 *
 * \param db The db to destroy
 */
void
fdb_database_release(fdb_database_t* db);

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
fdb_table_t*
fdb_database_create_table(fdb_database_t* db,
                          const char* name,
                          size_t csize, 
                          frs_dstr_t dstr);


/**
 * \brief Finds a table in the db
 *
 * \param db The db to find the table from
 * \param name The name of the db to get
 *
 * \return Returns a pointer to the table. nullptr if the table cannot be found
 */
fdb_table_t* 
fdb_database_find_table(fdb_database_t* db, 
                        const char* name);

/**
 * \brief Finds the table for the given name or creates if it does not exist
 *
 * \param db The db to find the table from
 * \param name The name of the table
 * \param csize The component size of the table
 * \param dstr The destructor for the components
 *
 * \return A pointer to the created table 
 */
fdb_table_t*
fdb_database_find_or_create_table(fdb_database_t* db,
                                  const char* name,
                                  size_t csize, 
                                  frs_dstr_t dstr);
/**
 * \brief Removes a table
 *
 * \tparam TComponent The component stored in the table
 * \param db The db to find the table from
 *
 */
void
fdb_database_remove_table(fdb_database_t* db, 
                          const char* name);

/**
 * \brief Clears and removes all the tables from the db
 *
 * \tparam TComponent The component stored in the table
 * \param db The db to find the table from
 *
 */
void 
fdb_database_clear(fdb_database_t* db);

/**
 * \brief Clears all the rows of a given element 
 *
 * \param id The id of the element to remove
 * \param db The db to find the table from
 *
 */
void
fdb_database_clear_entity(fdb_database_t* db, 
                          entity_id_t id);

/**
 * \brief Tags an entity with the given tag
 *
 * \param entity_id The id of the entity to tag
 * \param tag The tag to tag the entity with
 */
void 
fdb_database_tag_entity(fdb_database_t* db, 
                        entity_id_t entity_id, 
                        const char* tag);

/**
 * \brief Untags an entity from a given tag
 *
 * \param entity_id The entity to untag
 * \param tag The tag to remove from the entity
 */
void 
fdb_database_untag_entity(fdb_database_t* db, 
                          entity_id_t entity_id, 
                          const char* tag);

/**
 * \brief Gets a bitset with the entities with a given tag
 *
 * \param tag The tag to get the entities for
 * 
 * \return Returns a bitset with the entities of the tag
 */
fdb_bittable_t*
fdb_database_get_tagged_entities(fdb_database_t* db, 
                                 const char* tag);

/**
 * \brief Finds or Creates a reference table
 *
 * \param ref_name The reference name
 *
 * \return Returns a TableView of the reference table
 */
fdb_reftable_t*
fdb_database_find_or_create_reftable(fdb_database_t* db, 
                                     const char* ref_name);

/**
 * \brief  Finds a ref  table
 *
 * \param ref_name The reference table to find
 *
 * \return Returns a table view of the reference table
 */
fdb_reftable_t*
fdb_database_find_reftable(fdb_database_t* db, 
                           const char* ref_name);

/**
 * \brief Creates a ref  table
 *
 * \param ref_name The reference table to find
 *
 * \return Returns a table view of the reference table
 */
fdb_reftable_t*
fdb_database_create_reftable(fdb_database_t* db, 
                             const char* ref_name);


/**
 * \brief Starts a webserver to listen to the given address and port
 */
void
fdb_database_start_webserver(fdb_database_t* db, 
                             const char* address, 
                             const char* port);

/**
 * \brief Stops the web server
 */
void
fdb_database_stop_webserver(fdb_database_t* db);

/**
 * \brief locks the db for insertions and removal of tables
 */
void
fdb_database_lock(fdb_database_t* db);

/**
 * \brief unlocks the db for insertions and removal of tables
 */
void
fdb_database_unlock(fdb_database_t* db);

/**
 * \brief Gets the table metadata of the different tables in the db  
 *
 * \param data A pointer to TableInfo array 
 * \param capacity The capacity of the TableInfo array
 *
 * \return Returns the number of entries set in the array
 */
size_t 
fdb_database_metadata(fdb_database_t* db, 
                      fdb_table_info_t* data, 
                      uint32_t capacity);

/**
 * \brief Returns the number of tables in the db
 *
 * \return 
 */
size_t
fdb_database_num_tables(fdb_database_t* db);

/**
 * \brief Creates a global
 *
 * \param db The db to create the global to
 * \param name The name of the global
 * \param gsize The size in bytes of the global
 *
 * \return Returns a pointer to the created global. Returns nullptr if it 
 * cannot be created
 */
void*
fdb_database_create_global(fdb_database_t* db, 
                           const char* name,
                           size_t gsize,
                           frs_dstr_t dstr);


/**
 * \brief Removes an existing global
 *
 * \param db The db to remove the global from
 * \param name The name of the global to remove
 *
 * \tparam T The global to remove 
 */
void 
fdb_database_remove_global(fdb_database_t* db, 
                           const char* name);

/**
 * \brief Gets the global of a given type
 *
 * \param db The db to find the global into
 * \param name The name of the global 
 *
 * \return A pointer to the global. Returns nullptr if the global could not be
 * found
 */
void*
fdb_database_find_global(fdb_database_t* db, 
                         const char* name);

/**
 * \brief Gets the global of a given type. Non-locking version meant to be
 * used by the code generated by the compiler
 *
 * \param db The db to find the global into
 * \param name The name of the global 
 *
 * \return A pointer to the global. Returns nullptr if the global could not be
 * found
 */
void*
fdb_database_find_global_no_lock(fdb_database_t* db, 
                                 const char* name);

/**
 * \brief Finds or creates a new global
 *
 * \param db The db to create or find the global into 
 *
 * \return Returns a pointer to the global.
 */
void*
fdb_database_find_or_create_global(fdb_database_t* db,
                                   const char* name, 
                                   size_t gsize, 
                                   frs_dstr_t dstr);


/**
 * \brief Gets the metadata registry of the db
 *
 * \param db The db to get the registry from
 *
 * \return Returns the metadata registry
 */
fdb_mregistry_t*
fdb_database_get_mregistry(fdb_database_t* db);


uint32_t
get_table_id(const char* table_name);

#ifdef __cplusplus
}
#endif


#endif
