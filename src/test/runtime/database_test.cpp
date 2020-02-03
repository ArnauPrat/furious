
#include "furious.h"
#include <gtest/gtest.h>


FDB_BEGIN_COMPONENT(Component, KILOBYTES(4))

  uint32_t field1_;
  double field2_;

  Component(uint32_t field1, double field2) : 
    field1_(field1),
    field2_(field2)
  {}
FDB_END_COMPONENT

TEST(DatabaseTest, CreateAndRemoveTable) {
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_clear(&database);
  FDB_CREATE_TABLE(&database, Component, NULL);
  FDB_REMOVE_TABLE(&database, Component);
  fdb_database_release(&database);
}


TEST(DatabaseTest, FindTable) {
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_clear(&database);
  FDB_CREATE_TABLE(&database, Component, nullptr);
  FDB_FIND_TABLE(&database, Component);
  FDB_REMOVE_TABLE(&database, Component);
  fdb_database_release(&database);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
