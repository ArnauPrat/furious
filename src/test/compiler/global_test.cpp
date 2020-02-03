
#include "furious.h"
#include "global_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>

  
TEST(GlobalTest, GlobalTest ) 
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");

  fdb_table_t* pos_table = FDB_FIND_OR_CREATE_TABLE(&database, Position, nullptr);
  GlobalComponent* g = FDB_CREATE_GLOBAL(&database, GlobalComponent, nullptr);
  g->m_x = 1.0;
  g->m_y = 1.0;
  g->m_z = 1.0;

  entity_id_t NUM_ENTITIES = 8;
  for(uint32_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table, Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;
  }

  furious_init(&database);
  furious_frame(0.1, &database, nullptr);

  // FIRST LEVEL
  for(uint32_t i = 0; i < 8; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table,Position,i);
    ASSERT_EQ(pos->m_x, 1.0f);
    ASSERT_EQ(pos->m_y, 1.0f);
    ASSERT_EQ(pos->m_z, 1.0f);
  }

  furious_release();
  fdb_database_release(&database);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

