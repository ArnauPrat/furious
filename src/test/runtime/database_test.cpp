
#include "furious.h"
#include <gtest/gtest.h>

namespace furious {

FURIOUS_BEGIN_COMPONENT(Component, KILOBYTES(4))

  uint32_t field1_;
  double field2_;

  Component(uint32_t field1, double field2) : 
    field1_(field1),
    field2_(field2)
  {}
FURIOUS_END_COMPONENT

TEST(DatabaseTest, CreateAndRemoveTable) {
  database_t database = database_create();
  database_clear(&database);
  FURIOUS_CREATE_TABLE(&database, Component);
  FURIOUS_REMOVE_TABLE(&database, Component);
  database_destroy(&database);
}


TEST(DatabaseTest, FindTable) {
  database_t database = database_create();
  database_clear(&database);
  FURIOUS_CREATE_TABLE(&database, Component);
  FURIOUS_FIND_TABLE(&database, Component);
  FURIOUS_REMOVE_TABLE(&database, Component);
  database_destroy(&database);
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
