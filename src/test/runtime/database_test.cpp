
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
  fdb_tx_init(NULL);
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_clear(&database, 
                     &tx, 
                     txtctx);
  FDB_CREATE_TABLE(&database, &tx, txtctx, Component, NULL);
  FDB_REMOVE_TABLE(&database, &tx, txtctx, Component);
  fdb_database_release(&database, 
                       &tx, 
                       txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(DatabaseTest, FindTable) {
  fdb_tx_init(NULL);
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_clear(&database, 
                     &tx, 
                     txtctx);
  FDB_CREATE_TABLE(&database, &tx, txtctx, Component, nullptr);
  FDB_FIND_TABLE(&database, Component);
  FDB_REMOVE_TABLE(&database, &tx, txtctx, Component);
  fdb_database_release(&database, &tx, txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
