
#include "furious.h"
#include <gtest/gtest.h>

TEST(TagTests,TagWorks) 
{
  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);

  entity_id_t entX = 0;
  entity_id_t entY = 1;
  entity_id_t entZ = 2;

  fdb_bittable_t* selected = fdb_database_get_tagged_entities(&database, "selected");
  fdb_bittable_add(selected, entX);
  fdb_bittable_add(selected, entZ);

  ASSERT_TRUE(fdb_bittable_exists(selected, entX));
  ASSERT_FALSE(fdb_bittable_exists(selected, entY));
  ASSERT_TRUE(fdb_bittable_exists(selected, entZ));

  fdb_database_release(&database);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

