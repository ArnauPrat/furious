

#ifndef _FURIOUS_DATABASE_H_
#define _FURIOUS_DATABASE_H_

#include "table.h"
#include "common.h"
#include "../common/reflection.h"

#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <cassert>
#include <iostream>

namespace furious {

using TableMap = std::map<std::string, Table*>;
using TableMapPair = std::pair<std::string, Table*>;

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
  Table* create_table();

  /**
   * Drops an existing table
   */
  template <typename T>
  void drop_table();

  /**
   * Gets the table from type 
   * */
  template <typename T>
  Table* find_table();

  /**
   * Clears and removes all the tables from the database
   * */
  void clear();

  static Database* get_instance();

protected:
  Database() = default;

private:

  TableMap          m_tables;      /** Holds a map between component types and their tables **/
};

}

#include "database.inl"

#endif


