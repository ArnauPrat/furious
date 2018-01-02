

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "table.h"
#include "table_view.h"
#include "common.h"
#include "../common/reflection.h"

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
  TableView<T> create_table();

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
  Table* find_table(uint64_t id);

  uint64_t get_table_id(const std::string& name);

  template<typename TComponent>
    uint64_t get_table_id();

  /**
   * Clears and removes all the tables from the database
   * */
  void clear();

  static Database* get_instance();

protected:
  Database() = default;

private:

  std::map<uint64_t, Table*>  m_tables;      /** Holds a map between component types and their tables **/
};

}

#include "database.inl"

#endif


