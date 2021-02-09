
#include "furious.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>

  

TEST(BasicTest, BasicTest ) 
{
  fdb_tx_init(nullptr);
  fdb_database_t database;
  fdb_database_init(&database, NULL);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  
  fdb_txtable_t* pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_txtable_t* vel_table = FDB_FIND_TABLE(&database, Velocity);

  entity_id_t NUM_ENTITIES = 1000;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table, &tx, txtctx, Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;
    Velocity* vel = FDB_ADD_COMPONENT(vel_table, &tx, txtctx, Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f; 
    vel->m_z = 1.0f;
  }
  fdb_tx_commit(&tx);

  furious_frame(0.1,&database, nullptr);

  fdb_tx_begin(&tx, E_READ_ONLY);
  txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  for (entity_id_t i = 0; i < NUM_ENTITIES; ++i) 
  {
    Position* position = FDB_GET_COMPONENT(pos_table, &tx, txtctx, Position, i, false);
    ASSERT_EQ(position->m_x,-0.1f);
    ASSERT_EQ(position->m_y,-0.1f);
    ASSERT_EQ(position->m_z,-0.1f);
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

