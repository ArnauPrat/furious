

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "reflection.h"
#include "table.h"
#include "bit_table.h"
#include "macros.h"
#include "table_view.h"
#include "../../common/platform.h"
#include "../../common/btree.h"
#include "../../common/mutex.h"

#include <assert.h>

template<typename T>
void destructor(void *ptr) 
{
  static_cast<T*>(ptr)->~T();
}

template<typename T, void (*Func)(T*)>
void destructor_manual(void *ptr) 
{
  Func((T*)ptr);
}


namespace furious 
{

struct WebServer;
struct TableInfo
{
  char        m_name[FURIOUS_MAX_TABLE_NAME];
  uint32_t    m_size;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct GlobalInfo
{
  FURIOUS_ALIGNED(void*, p_global, 64);
  void (*m_destructor)(void *ptr);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * \brief Stores all the tables and globals
 */
struct database_t 
{
  btree_t                       m_tags;               //< btree with the bittable holding the entities with tags 
  btree_t                       m_tables;             //< btree with the component tables 
  btree_t                       m_references;         //< btree with the reference tables 
  btree_t                       m_temp_tables;        //< btree with the temporal tables 
  btree_t                       m_globals;            //< btree with the globals 
  btree_t                       m_refl_data;          //< btree with the reflection data 
  entity_id_t                   m_next_entity_id;     //< The next entity id to assign
  WebServer*                    p_webserver;          //< The database web server
  mutex_t                       m_mutex;              //< The mutex to exclusive access to the database

  mem_allocator_t               m_page_allocator;     //< Allocator used to allocate pages for the rest of allocators
  mem_allocator_t               m_table_allocator;    //< Allocator for tables, temporal tables and reference tables
  mem_allocator_t               m_bittable_allocator; //< Allocator for bittables
  mem_allocator_t               m_btree_allocator;    //< Allocator for database's btree nodes
  mem_allocator_t               m_global_allocator;   //< Allocator for globals
};


/**
 * \brief Creates a furious database
 *
 * \return The created database
 */
database_t
database_create(mem_allocator_t* allocator = nullptr);

/**
 * \brief Destroys a database
 *
 * \param database The database to destroy
 */
void
database_destroy(database_t* database);

/**
 * \brief Adds a table to the database 
 *
 * \tparam T The component to store in the created table
 * \tparam (*fp)(T*) The destructor to use when deleting the elements of the
 * table
 * \param database The database to create the table to
 *
 * \return Returns a view to the table
 */
template <typename T, void (*fp)(T*)>
TableView<T>
database_create_table(database_t* database);

/**
 * \brief Adds a table to the database 
 *
 * \tparam T The component to store in the created table
 * \param database The database to create the table to
 *
 * \return Returns a view to the table
 */
template <typename T>
TableView<T>
database_create_table(database_t* database);


/**
 * \brief Finds a table in the database
 *
 * \tparam T The component of the table to find
 * \param database The database to find the table from
 *
 * \return Returns a view to the table
 */
template <typename T>
TableView<T> 
database_find_table(database_t* database);

/**
 * \brief Finds the table for the specified component or adds it to the
 * database if it does not exist
 *
 * \tparam T The component type
 * \param database The database to find the table from
 *
 * \return A TableView of the table
 */
template <typename T>
TableView<T> 
database_find_or_create_table(database_t* database);

/**
 * \brief Finds the table for the specified component or adds it to the
 * database if it does not exist
 *
 * \tparam T The component type
 * \param database The database to find the table from
 *
 * \return A TableView of the table
 */
template <typename T, void (*fp)(T*)>
TableView<T> 
database_find_or_create_table(database_t* database);


/**
 * \brief Tests whether a Table exists or not
 *
 * \tparam TComponent The component stored in the table
 * \param database The database to find the table from
 *
 * \return Returns True if the table exists. False otherwise
 */
template <typename T>
bool 
database_exists_table(database_t* database);

/**
 * \brief Removes a table
 *
 * \tparam TComponent The component stored in the table
 * \param database The database to find the table from
 *
 */
template <typename T>
void
database_remove_table(database_t* database);

/**
 * \brief Clears and removes all the tables from the database
 *
 * \tparam TComponent The component stored in the table
 * \param database The database to find the table from
 *
 */
void 
database_clear(database_t* database);

/**
 * \brief Clears all the rows of a given element 
 *
 * \param id The id of the element to remove
 * \param database The database to find the table from
 *
 */
void
database_clear_entity(database_t* database, 
                      entity_id_t id);

/**
 * \brief Gets the next free entity id
 *
 * \return Returns the next free entity id
 */
entity_id_t 
database_get_next_entity_id(database_t* database);

/**
 * \brief Tags an entity with the given tag
 *
 * \param entity_id The id of the entity to tag
 * \param tag The tag to tag the entity with
 */
void 
database_tag_entity(database_t* database, 
                    entity_id_t entity_id, 
                    const char* tag);

/**
 * \brief Untags an entity from a given tag
 *
 * \param entity_id The entity to untag
 * \param tag The tag to remove from the entity
 */
void 
database_untag_entity(database_t* database, 
                      entity_id_t entity_id, 
                      const char* tag);

/**
 * \brief Gets a bitset with the entities with a given tag
 *
 * \param tag The tag to get the entities for
 * 
 * \return Returns a bitset with the entities of the tag
 */
BitTable*
database_get_tagged_entities(database_t* database, 
                             const char* tag);

/**
 * \brief Finds or Creates a reference table
 *
 * \param ref_name The reference name
 *
 * \return Returns a TableView of the reference table
 */
TableView<uint32_t>
database_find_or_create_ref_table(database_t* database, 
                                  const char* ref_name);

/**
 * \brief  Finds a ref  table
 *
 * \param ref_name The reference table to find
 *
 * \return Returns a table view of the reference table
 */
TableView<uint32_t>
database_find_ref_table(database_t* database, 
                        const char* ref_name);

/**
 * \brief Creates a ref  table
 *
 * \param ref_name The reference table to find
 *
 * \return Returns a table view of the reference table
 */
TableView<uint32_t>
database_create_ref_table(database_t* database, 
                          const char* ref_name);

/**
 * \brief Adds a reference between two entities
 *
 * \param type The type of the reference
 * \param tail The origin entity of the reference
 * \param head The destination entity of the reference
 */
void 
database_add_reference(database_t* database, 
                       const char* type, 
                       entity_id_t tail, 
                       entity_id_t head);

/**
 * \brief Removes a reference between two entities
 *
 * \param type The type of the reference
 * \param tail The origin entity of the reference
 */
void 
database_remove_reference(database_t* database, 
                          const char* type, 
                          entity_id_t tail);

  /**
   * \brief Starts a webserver to listen to the given address and port
   */
void
database_start_webserver(database_t* database, 
                         const char* address, 
                         const char* port);

/**
 * \brief Stops the web server
 */
void
database_stop_webserver(database_t* database);

/**
 * \brief locks the database for insertions and removal of tables
 */
void
database_lock(database_t* database);

/**
 * \brief unlocks the database for insertions and removal of tables
 */
void
database_release(database_t* database);

/**
 * \brief Gets the table metadata of the different tables in the Database  
 *
 * \param data A pointer to TableInfo array 
 * \param capacity The capacity of the TableInfo array
 *
 * \return Returns the number of entries set in the array
 */
size_t 
database_metadata(database_t* database, 
                  TableInfo* data, 
                  uint32_t capacity);

/**
 * \brief Returns the number of tables in the database
 *
 * \return 
 */
size_t
database_num_tables(database_t* database);


/**
 * \brief Creates a temporal table for the given component type
 *
 * \tparam T The type of the components stored in the table
 *
 * \return A TableView of the temporal table
 */
template <typename T>
TableView<T>
database_create_temp_table(database_t* database, 
                           const char* table_name);

/**
 * \brief Creates a temporal table for the given component type (withput locking the database). Meant to
 * be used only in fcc generated code.
 *
 * \tparam T The type of the components stored in the table
 *
 * \return A TableView of the temporal table
 */
template <typename T>
TableView<T>
database_create_temp_table_no_lock(database_t* database, 
                                   const char* table_name);

/**
 * \brief Destroys the given temporal table
 *
 * \tparam T The type of the componnets stored in the table
 * \param view The view to the temporal table to destroy
 */
template <typename T>
void
database_remove_temp_table(database_t* database, 
                           TableView<T> view);

/**
 * \brief Destroys the given temporal table (withput locking the database). Meant to
 * be used only in fcc generated code.
 *
 * \tparam T The type of the componnets stored in the table
 * \param view The view to the temporal table to destroy
 */
template <typename T>
void
database_remove_temp_table_no_lock(database_t* database,
                                   TableView<T> view);

/**
 * \brief Remove all temporal tables
 */
void
database_remove_temp_tables(database_t* database);

/**
 * \brief Removes all temporal tables (without locking the database). Meant to
 * be used only in fcc generated code.
 */
void
database_remove_temp_tables_no_lock(database_t* database);

/**
 * \brief Creates a new global
 *
 * \tparam T The global to create
 *
 * \return Returns a pointer to the newly created global or nullptr if already
 * exists
 */
template <typename T,typename...Args>
T*
database_create_global(database_t* database, 
                       Args...args);


/**
 * \brief Removes an existing global
 *
 * \tparam T The global to remove 
 */
template <typename T>
void 
database_remove_global(database_t* database);

/**
 * \brief Gets the global of a given type
 *
 * \tparam T The type of the global
 *
 * \return A pointer to the global
 */
template <typename T>
T* 
database_find_global(database_t* database);

/**
 * \brief Gets the global of a given type. Non-locking version meant to be
 * used by the code generated by the compiler
 *
 * \tparam T The type of the global
 *
 * \return A pointer to the global
 */
template <typename T>
T* 
database_find_global_no_lock(database_t* database);

/**
 * \brief Finds or creates a new global
 *
 * \tparam T The type of the global to create
 *
 * \return 
 */
template <typename T,typename...Args>
T* 
database_find_or_create_global(database_t* database, 
                               Args...args);

/**
 * \brief Tests whether a global exists or not
 *
 * \return Returns True if the global exists. False otherwise
 */
template <typename T>
bool 
database_exists_global(database_t* database);


/**
 * \brief Adds reflection data for a given table or global 
 *
 * \param refl_strct The reflection data to add
 */
template <typename T>
void
database_add_refl_data(database_t* database, 
                       RefCountPtr<ReflData> refl_strct);


/**
 * \brief Gets the reflection data associated to a given table 
 *
 * \tparam T
 */
template <typename T>
const ReflData* 
database_get_refl_data(database_t* database);

uint32_t
get_table_id(const char* table_name);

}

#endif

#include "database.inl"


