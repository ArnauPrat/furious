
#include "furious.h"
#include <gtest/gtest.h>

TEST(TagTests,TagWorks) 
{
  fdb_tx_init(NULL);
  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);

  entity_id_t entX = 0;
  entity_id_t entY = 1;
  entity_id_t entZ = 2;

  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbittable_t* selected = fdb_database_get_tagged_entities(&database, 
                                                                &tx, 
                                                                txtctx, 
                                                                "selected");
  fdb_txbittable_add(selected, &tx, txtctx, entX);
  fdb_txbittable_add(selected, &tx, txtctx, entZ);

  ASSERT_TRUE(fdb_txbittable_exists(selected, &tx, txtctx, entX));
  ASSERT_FALSE(fdb_txbittable_exists(selected,&tx, txtctx,  entY));
  ASSERT_TRUE(fdb_txbittable_exists(selected, &tx, txtctx, entZ));

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

