
#include "furious.h"
#include <gtest/gtest.h>

namespace furious {

struct Component {
  uint32_t field1_;
  double field2_;
  Component(uint32_t field1, double field2) : 
    field1_(field1),
    field2_(field2)
  {}

  static std::string name() { return "Component"; }
};

TEST(DatabaseTest, CreateAndRemoveTable) {
  auto database = new Database();
  database->clear();
  auto table = FURIOUS_CREATE_TABLE(database, Component);
  FURIOUS_REMOVE_TABLE(database, Component);
  delete database;
}


TEST(DatabaseTest, FindTable) {
  auto database = new Database();
  database->clear();
  FURIOUS_CREATE_TABLE(database, Component);
  auto table = FURIOUS_FIND_TABLE(database, Component);
  FURIOUS_REMOVE_TABLE(database, Component);
  delete database;
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
