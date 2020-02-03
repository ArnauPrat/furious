
#include "furious.h"
#include "filter_func_test_header.h"

#include <gtest/gtest.h>


TEST(FilterFuncTest, FilterFuncTest ) 
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
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
    float tmp = 1.0f;
    if(i % 2 != 0)
    {
      tmp = 2.0f;
    }

    vel->m_x = tmp;
    vel->m_y = tmp;
    vel->m_z = tmp;
  }

  furious_frame(0.1f, &database, nullptr);

  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, Position, i);

    float tmp = 0.1;
    if(i % 2 != 0)
    {
      tmp = 0.0;
    } 
    
    ASSERT_EQ(pos->m_x, tmp);
    ASSERT_EQ(pos->m_y, tmp);
    ASSERT_EQ(pos->m_z, tmp);

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

