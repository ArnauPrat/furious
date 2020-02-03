
#include "furious.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>

  

TEST(BasicTest, BasicTest ) 
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  fdb_table_t* pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_table_t* vel_table = FDB_FIND_TABLE(&database, Velocity);

  entity_id_t NUM_ENTITIES = 1000;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table, Position, i);
    pos->m_x = 0.0;
    pos->m_y = 0.0;
    pos->m_z = 0.0;

    Velocity* vel = FDB_ADD_COMPONENT(vel_table, Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f;
    vel->m_z = 1.0f;
  }

  furious_frame(0.1, &database, nullptr);

  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, Position, i);
    ASSERT_EQ(pos->m_x,0.1f);
    ASSERT_EQ(pos->m_y,0.1f);
    ASSERT_EQ(pos->m_z,0.1f);
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

