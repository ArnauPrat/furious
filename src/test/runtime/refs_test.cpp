
#include "furious.h"
#include <gtest/gtest.h>

FDB_BEGIN_COMPONENT(ComponentA, KILOBYTES(4))
  uint32_t m_field;
FDB_END_COMPONENT

FDB_BEGIN_COMPONENT(ComponentB, KILOBYTES(4))
  uint32_t m_field;
FDB_END_COMPONENT

TEST(RefsTest,TagWorks) 
{
  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);

  entity_id_t entX=0;
  entity_id_t entY=1;
  entity_id_t entZ=2;

  fdb_reftable_t* rt = fdb_database_find_or_create_reftable(&database, "test_ref");
  fdb_reftable_add(rt, entX, entY);
  fdb_reftable_add(rt, entY, entZ);


  ASSERT_TRUE(fdb_reftable_exists(rt, entX, entY));
  ASSERT_TRUE(fdb_reftable_exists(rt, entY, entZ));
  ASSERT_FALSE(fdb_reftable_exists(rt, entX, entZ));

  fdb_reftable_add(rt, entX, entZ);

  ASSERT_TRUE(fdb_reftable_exists(rt, entX, entZ));
  ASSERT_FALSE(fdb_reftable_exists(rt, entX, entY));

  fdb_database_release(&database);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

