
#include "../data/database.h"
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

template<>
std::string type_name<Component>() {
  return "Component";
}

TEST(DatabaseTest, CreateAndRemoveTable) {
  auto database = Database::get_instance();
  database->clear();
  auto table = database->create_table<Component>();
  database->remove_table<Component>();
}


TEST(DatabaseTest, FindTable) {
  auto database = Database::get_instance();
  database->clear();
  database->create_table<Component>();
  auto table = database->find_table<Component>();
  database->remove_table<Component>();
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}