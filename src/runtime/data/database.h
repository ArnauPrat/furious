

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "reflection.h"
#include "table.h"
#include "bit_table.h"
#include "macros.h"
#include "table_view.h"
#include "../../common/types.h"
#include "../../common/hash_map.h"

#include <assert.h>
#include <mutex>

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

#define _FURIOUS_INVALID_ENTITY_ID 0xffffffff
#define _FURIOUS_TABLE_INFO_MAX_NAME_LENGTH 128

struct WebServer;

struct TableInfo
{
  char        m_name[_FURIOUS_TABLE_INFO_MAX_NAME_LENGTH];
  uint32_t    m_size;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct GlobalInfo
{
  void * p_global;
  void (*m_destructor)(void *ptr);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


struct Database 
{
  Database();
  Database( const Database& ) = delete;
  Database( Database&& ) = delete;

  virtual ~Database();

  Database& operator=( const Database& ) = delete;
  Database& operator=( Database&& ) = delete;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  /**
   * Adds a table to the database
   */
  template <typename T, void (*fp)(T*)>
  TableView<T>
  create_table();

  /**
   * Adds a table to the database
   */
  template <typename T>
  TableView<T>
  create_table();

  /**
   * Drops an existing table
   */
  template <typename T>
  void 
  remove_table();

  /**
   * Gets the table from type 
   * */
  template <typename T>
  TableView<T> 
  find_table();

  /**
   * @brief Finds the table for the specified component or adds it to the
   * database if it does not exist
   *
   * @tparam T The component type
   *
   * @return A TableView of the table
   */
  template <typename T>
  TableView<T> 
  find_or_create_table();

  /**
   * @brief Finds the table for the specified component or adds it to the
   * database if it does not exist
   *
   * @tparam T The component type
   *
   * @return A TableView of the table
   */
  template <typename T, void (*fp)(T*)>
  TableView<T> 
  find_or_create_table();

  /**
   * @brief Tests whether a Table exists or not
   *
   * @tparam TComponent The component stored in the table
   *
   * @return Returns True if the table exists. False otherwise
   */
  template<typename T>
  bool 
  exists_table();

  /**
   * \brief Gets the id of a table 
   *
   * \param tablename The name of a table
   *
   * \return Returns the id of the table
   */
  template <typename T>
  uint32_t
  get_table_id();

  /**
   * Clears and removes all the tables from the database
   * */
  void 
  clear();

  /**
   * Clears all the rows of a given element 
   *
   * @param id The id of the element to remove
   */
  void
  clear_entity(entity_id_t id);

  /**
   * @brief Gets the next free entity id
   *
   * @return Returns the next free entity id
   */
  entity_id_t 
  get_next_entity_id();

  /**
   * @brief Tags an entity with the given tag
   *
   * @param entity_id The id of the entity to tag
   * @param tag The tag to tag the entity with
   */
  void 
  tag_entity(entity_id_t entity_id, const char* tag);

  /**
   * @brief Untags an entity from a given tag
   *
   * @param entity_id The entity to untag
   * @param tag The tag to remove from the entity
   */
  void 
  untag_entity(entity_id_t entity_id, const char* tag);

  /**
   * @brief Gets a bitset with the entities with a given tag
   *
   * @param tag The tag to get the entities for
   * 
   * @return Returns a bitset with the entities of the tag
   */
  BitTable*
  get_tagged_entities(const char* tag);

  /**
   * \brief Gets a reference table
   *
   * \param ref_name The reference name
   *
   * \return Returns a TableView of the reference table
   */
  TableView<uint32_t>
  get_references(const char* ref_name);

  /**
   * @brief Adds a reference between two entities
   *
   * @param type The type of the reference
   * @param tail The origin entity of the reference
   * @param head The destination entity of the reference
   */
  void 
  add_reference( const char* type, 
                 entity_id_t tail, 
                 entity_id_t head);

  /**
   * @brief Removes a reference between two entities
   *
   * @param type The type of the reference
   * @param tail The origin entity of the reference
   */
  void 
  remove_reference( const char* type, 
                    entity_id_t tail);

  /**
   * \brief Starts a webserver to listen to the given address and port
   *
   */
  void
  start_webserver(const char* address, 
                  const char* port);

  /**
   * \brief Stops the web server
   */
  void
  stop_webserver();

  /**
   * \brief locks the database for insertions and removal of tables
   */
  void
  lock() const;

  /**
   * \brief unlocks the database for insertions and removal of tables
   */
  void
  release() const;

  /**
   * \brief Gets the table metadata of the different tables in the Database  
   *
   * \param data A pointer to TableInfo array 
   * \param capacity The capacity of the TableInfo array
   *
   * \return Returns the number of entries set in the array
   */
  size_t 
  meta_data(TableInfo* data, uint32_t capacity);

  /**
   * \brief Returns the number of tables in the database
   *
   * \return 
   */
  size_t
  num_tables() const;


  /**
   * \brief Creates a temporal table for the given component type
   *
   * @tparam T The type of the components stored in the table
   *
   * \return A TableView of the temporal table
   */
  template <typename T>
  TableView<T>
  create_temp_table(const char* table_name);

  /**
   * \brief Creates a temporal table for the given component type (withput locking the database). Meant to
   * be used only in fcc generated code.
   *
   * @tparam T The type of the components stored in the table
   *
   * \return A TableView of the temporal table
   */
  template <typename T>
  TableView<T>
  create_temp_table_no_lock(const char* table_name);

  /**
   * \brief Destroys the given temporal table
   *
   * @tparam T The type of the componnets stored in the table
   * \param view The view to the temporal table to destroy
   */
  template <typename T>
  void
  remove_temp_table(TableView<T> view);

  /**
   * \brief Destroys the given temporal table (withput locking the database). Meant to
   * be used only in fcc generated code.
   *
   * @tparam T The type of the componnets stored in the table
   * \param view The view to the temporal table to destroy
   */
  template <typename T>
  void
  remove_temp_table_no_lock(TableView<T> view);

  /**
   * \brief Remove all temporal tables
   */
  void
  remove_temp_tables();

  /**
   * \brief Removes all temporal tables (without locking the database). Meant to
   * be used only in fcc generated code.
   */
  void
  remove_temp_tables_no_lock();

  /**
   * \brief Creates a new global
   *
   * @tparam T The global to create
   *
   * \return Returns a pointer to the newly created global or nullptr if already
   * exists
   */
  template <typename T,typename...Args>
  T*
  create_global(Args...args);

  /**
   * \brief Removes an existing global
   *
   * @tparam T The global to remove 
   */
  template <typename T>
  void 
  remove_global();

  /**
   * \brief Gets the global of a given type
   *
   * @tparam T The type of the global
   *
   * \return A pointer to the global
   */
  template <typename T>
  T* 
  find_global();

  /**
   * \brief Gets the global of a given type. Non-locking version meant to be
   * used by the code generated by the compiler
   *
   * @tparam T The type of the global
   *
   * \return A pointer to the global
   */
  template <typename T>
  T* 
  find_global_no_lock();

  /**
   * \brief Finds or creates a new global
   *
   * @tparam T The type of the global to create
   *
   * \return 
   */
  template <typename T,typename...Args>
  T* 
  find_or_create_global(Args...args);

  /**
   * \brief Tests whether a global exists or not
   *
   * \tparam TComponent The global stored in the table
   *
   * \return Returns True if the global exists. False otherwise
   */
  template<typename T>
  bool 
  exists_global();


  /**
   * \brief Adds reflection data for a given table or global 
   *
   * \param refl_strct The reflection data to add
   */
  template<typename T>
  void
  add_refl_data(RefCountPtr<ReflData> refl_strct);


  /**
   * \brief Gets the reflection data associated to a given table 
   *
   * @tparam T
   */
  template<typename T>
  const ReflData* 
  get_refl_data();

private:

  BTree<BitTable*>              m_tags;
  BTree<Table*>                 m_tables;           /** Holds a map between component types and their tables **/
  BTree<Table*>                 m_references;
  BTree<Table*>                 m_temp_tables;
  BTree<GlobalInfo>             m_globals;
  BTree<RefCountPtr<ReflData>>  m_refl_data;
  entity_id_t                   m_next_entity_id;
  WebServer*                    p_webserver;
  mutable std::mutex            m_mutex;
};

}

#include "database.inl"

#endif


