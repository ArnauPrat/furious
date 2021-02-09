
#include "furious.h"
#include "filter_func_test_header.h"

#include <gtest/gtest.h>


TEST(FilterFuncTest, FilterFuncTest ) 
{
  fdb_tx_init(NULL);
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  furious_init(&database);

  fdb_txtable_t* pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_txtable_t* vel_table = FDB_FIND_TABLE(&database, Velocity);

  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  entity_id_t NUM_ENTITIES = 1000;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table, &tx, txtctx, Position, i);
    pos->m_x = 0.0;
    pos->m_y = 0.0;
    pos->m_z = 0.0;

    Velocity* vel = FDB_ADD_COMPONENT(vel_table, &tx, txtctx, Velocity, i);
    float tmp = 1.0f;
    if(i % 2 != 0)
    {
      tmp = 2.0f;
    }

    vel->m_x = tmp;
    vel->m_y = tmp;
    vel->m_z = tmp;
  }
  fdb_tx_commit(&tx);

  furious_frame(0.1f, &database, nullptr);

  fdb_tx_begin(&tx, E_READ_ONLY);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, &tx, txtctx, Position, i, false);

    float tmp = 0.1;
    if(i % 2 != 0)
    {
      tmp = 0.0;
    } 
    
    ASSERT_EQ(pos->m_x, tmp);
    ASSERT_EQ(pos->m_y, tmp);
    ASSERT_EQ(pos->m_z, tmp);

  }
  fdb_tx_commit(&tx);
  furious_release();
  fdb_database_release(&database);
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

