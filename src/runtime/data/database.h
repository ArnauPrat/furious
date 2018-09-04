

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "table.h"
#include "table_view.h"
#include "common.h"
#include "../../common/traits.h"
#include "../../common/optional.h"

#include <bitset>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>

namespace furious {

class Database final {
public:
  Database() = default;
  Database( const Database& ) = delete;
  Database( Database&& ) = delete;

  virtual ~Database();

  Database& operator=( const Database& ) = delete;
  Database& operator=( Database&& ) = delete;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  /**
   * Adds an existing table to the database
   */
  template <typename T>
  TableView<T> add_table();

  /**
   * Drops an existing table
   */
  template <typename T>
  void remove_table();

  /**
   * Gets the table from type 
   * */
  template <typename T>
  TableView<T> find_table();

  /**
   * @brief Gets the id of a table
   *
   * @tparam TComponent
   *
   * @return 
   */
  template<typename TComponent>
    int64_t get_table_id();


  /**
   * @brief Tests whether a Table exists or not
   *
   * @tparam TComponent The component stored in the table
   *
   * @return Returns True if the table exists. False otherwise
   */
  template<typename TComponent>
    bool exists_table();

  /**
   * Clears and removes all the tables from the database
   * */
  void clear();

  /**
   * Clears all the rows of a given element 
   *
   * @param id The id of the element to remove
   */
  void clear_element(int32_t id);


  /**
   * @brief Gets the next free entity id
   *
   * @return Returns the next free entity id
   */
  int32_t get_next_entity_id();

  /**
   * @brief Tags an entity with the given tag
   *
   * @param entity_id The id of the entity to tag
   * @param tag The tag to tag the entity with
   */
  void tag_entity(int32_t entity_id, const std::string& tag);

  /**
   * @brief Untags an entity from a given tag
   *
   * @param entity_id The entity to untag
   * @param tag The tag to remove from the entity
   */
  void untag_entity(int32_t entity_id, const std::string& tag);

  /**
   * @brief Gets a bitset with the entities with a given tag
   *
   * @param tag The tag to get the entities for
   * 
   * @return Returns a bitset with the entities of the tag
   */
  optional<const std::set<int32_t>*> 
    get_tagged_entities(const std::string& tag);


  /**
   * @brief Adds a reference between two entities
   *
   * @param type The type of the reference
   * @param tail The origin entity of the reference
   * @param head The destination entity of the reference
   */
  void add_reference( const std::string& type, 
                      int32_t tail, 
                      int32_t head);

  /**
   * @brief Removes a reference between two entities
   *
   * @param type The type of the reference
   * @param tail The origin entity of the reference
   * @param head The destination entity of the reference
   */
  void remove_reference( const std::string& type, 
                          int32_t tail, 
                          int32_t head);

private:

  std::map<std::string, std::set<int32_t>*> m_tags;
  std::map<int64_t, Table*>                 m_tables;           /** Holds a map between component types and their tables **/
  std::map<std::string, Table*>             m_references;
  int32_t                                   m_next_entity_id;
};

}

#include "database.inl"

#endif

