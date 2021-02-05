
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
  fdb_tx_init(NULL);
  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);

  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  entity_id_t entX=0;
  entity_id_t entY=1;
  entity_id_t entZ=2;

  fdb_txtable_t* rt = fdb_database_find_or_create_reftable(&database, 
                                                           &tx, 
                                                           txtctx, 
                                                           "test_ref");
  fdb_txtable_add_reference(rt, &tx, txtctx, entX, entY);
  fdb_txtable_add_reference(rt, &tx, txtctx, entY, entZ);


  ASSERT_TRUE(fdb_txtable_exists_reference(rt, &tx, txtctx, entX, entY));
  ASSERT_TRUE(fdb_txtable_exists_reference(rt, &tx, txtctx, entY, entZ));
  ASSERT_FALSE(fdb_txtable_exists_reference(rt,&tx, txtctx,  entX, entZ));

  fdb_txtable_add_reference(rt, 
                            &tx, 
                            txtctx, 
                            entX, entZ);

  ASSERT_TRUE(fdb_txtable_exists_reference(rt, &tx, txtctx, entX, entZ));
  ASSERT_FALSE(fdb_txtable_exists_reference(rt, &tx, txtctx, entX, entY));

  fdb_database_release(&database, 
                       &tx, 
                       txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

