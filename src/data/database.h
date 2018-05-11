

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "bitset.h"
#include "table.h"
#include "table_view.h"
#include "common.h"
#include "../common/reflection.h"
#include "../common/optional.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <cassert>
#include <iostream>

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
   * Gets a table from its name 
   * */
  Table*  find_table(const std::string& table_name);

  /**
   * Gets a table from its id
   */
  Table* find_table(int64_t id);

  int64_t get_table_id(const std::string& name);

  template<typename TComponent>
    int64_t get_table_id();

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

  void tag_entity(int32_t entity_id, const std::string& tag);

  void untag_entity(int32_t entity_id, const std::string& tag);

  optional<const bitset&> 
    get_tagged_entities(const std::string& tag);

private:

  std::map<std::string, bitset> m_tags;
  std::map<int64_t, Table*>     m_tables;      /** Holds a map between component types and their tables **/
  int32_t                       m_next_entity_id;
};

}

#include "database.inl"

#endif


