
#include <gtest/gtest.h>
#include "../data/database.h"

namespace furious {

struct ComponentA {
  uint32_t field1_;
  double field2_;
  ComponentA(uint32_t field1, double field2 ) : 
    field1_(field1),
    field2_(field2) 
  {}

  static std::string name() { return "ComponentA"; }
};

struct ComponentB {
  uint32_t field1_;
  double field2_;
  ComponentB(uint32_t field1, double field2 ) : 
    field1_(field1),
    field2_(field2) 
  {}

  static std::string name() { return "ComponentB"; }
};

template<>
std::string type_name<ComponentA>() {
  return "ComponentA";
}

template<>
std::string type_name<ComponentB>() {
  return "ComponentB";
}

class DataTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    database_ = Database::get_instance();
    tableA_ = database_->create_table<ComponentA>();
    tableB_ = database_->create_table<ComponentB>();
  }

  virtual void TearDown() {
    database_->drop_table<ComponentA>();
    database_->drop_table<ComponentB>();
    tableA_ = nullptr;
    tableB_ = nullptr;
  }

  Database* database_ = nullptr;
  Table* tableA_ = nullptr;
  Table* tableB_ = nullptr;

};

} /* furious */ 
