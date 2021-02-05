
#include "furious.h"

#include <gtest/gtest.h>
#include <set>

FDB_BEGIN_COMPONENT(Component, KILOBYTES(4))

  int32_t field1_;
  float field2_;

  Component(int32_t field1, double field2) : field1_(field1), field2_(field2) {}

FDB_END_COMPONENT

TEST(TableTest,TableWorks) 
{

  fdb_tx_init(NULL);

  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txtable_t* t = FDB_CREATE_TABLE(&database, &tx, txtctx, Component, nullptr);
  entity_id_t num_entities = FDB_TXTABLE_BLOCK_SIZE*2048;

  for(entity_id_t i = 0; i < num_entities; ++i) 
  {
    Component* c = FDB_ADD_COMPONENT(t, &tx, txtctx, Component, i);
    c->field1_ = i;
    c->field2_ = i;
  }

  ASSERT_EQ(fdb_txtable_size(t, &tx, txtctx), num_entities);

  fdb_txtable_iter_t iter;
  fdb_txtable_iter_init(&iter, t, &tx, txtctx, 1, 0, 1, false);
  int32_t counter = 0;
  while (fdb_txtable_iter_has_next(&iter)) 
  {
    fdb_txtable_block_t* block = fdb_txtable_iter_next(&iter);
    Component* data = (Component*)fdb_txpool_alloc_ptr(t->p_data_allocator, 
                                      &tx, 
                                      txtctx, 
                                      block->m_data,
                                      false);
    fdb_txbitmap_t* mask = &block->m_enabled;
    for (size_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i) 
    {
      ASSERT_TRUE(fdb_txbitmap_is_set(mask, &tx, txtctx, i));
      ASSERT_EQ(data[i].field1_, counter);
      ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
      counter++;
    }
  }
  fdb_txtable_iter_release(&iter);
  ASSERT_EQ(counter, num_entities);
  ASSERT_EQ(fdb_txtable_size(t, &tx, txtctx), num_entities);

  fdb_txtable_disable_component(t, &tx, txtctx, num_entities-1);
  ASSERT_FALSE(fdb_txtable_is_enabled(t, &tx, txtctx, num_entities-1));
  fdb_txtable_enable_component(t, &tx, txtctx, num_entities-1);
  ASSERT_TRUE(fdb_txtable_is_enabled(t, &tx, txtctx, num_entities-1));

  for(entity_id_t i = 0; i < num_entities; i+=2) {
    FDB_REMOVE_COMPONENT(t, &tx, txtctx, i);
  }
  ASSERT_EQ(fdb_txtable_size(t, &tx, txtctx), num_entities/2);

  fdb_txtable_iter_t iter2;
  fdb_txtable_iter_init(&iter2, t, &tx, txtctx, 1, 0, 1, false);
  int32_t num_real = 0;
  counter = 0;
  while (fdb_txtable_iter_has_next(&iter2)) 
  {
    fdb_txtable_block_t* block = fdb_txtable_iter_next(&iter2);
    fdb_txbitmap_t* mask = &block->m_enabled;
    Component* data = (Component*)fdb_txpool_alloc_ptr(t->p_data_allocator, 
                                           &tx, 
                                           txtctx,
                                           block->m_data, 
                                           false);
    for (size_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i) 
    {
      if(i % 2 == 0) 
      {
        ASSERT_FALSE(fdb_txbitmap_is_set(mask,&tx, txtctx, i));
      } 
      else 
      {
        ASSERT_TRUE(fdb_txbitmap_is_set(mask, &tx, txtctx, i));
        ASSERT_EQ(data[i].field1_, counter);
        ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
        num_real++;
      }
        counter++;
    }
  }
  fdb_txtable_iter_release(&iter2);
  ASSERT_EQ(num_real, num_entities/2);

  fdb_txtable_clear(t, &tx, txtctx);
  ASSERT_EQ(fdb_txtable_size(t, &tx, txtctx),0);

  FDB_REMOVE_TABLE(&database, &tx, txtctx, Component);
  fdb_database_release(&database, &tx, txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(IteratorTest,TableWorks) 
{

  fdb_tx_init(NULL);

  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txtable_t* t = FDB_CREATE_TABLE(&database, &tx, txtctx, Component, nullptr);
  int32_t num_entities = FDB_TXTABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_entities; ++i) 
  {
    Component* c = FDB_ADD_COMPONENT(t, &tx, txtctx, Component, i);
    c->field1_ = i;
    c->field2_ = i;
  }

  fdb_txtable_iter_t it;
  fdb_txtable_iter_init(&it, 
                        t, 
                        &tx, 
                        txtctx, 
                        1, 0, 2, 
                        false);
  while(fdb_txtable_iter_has_next(&it))
  {
    fdb_txtable_block_t* block = fdb_txtable_iter_next(&it);
    uint32_t block_start = block->m_start / FDB_TXTABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 0);
  }
  fdb_txtable_iter_release(&it);

  fdb_txtable_iter_t it2;
  fdb_txtable_iter_init(&it2, t, &tx, txtctx, 1, 1, 2, false);
  while(fdb_txtable_iter_has_next(&it2))
  {
    fdb_txtable_block_t* block = fdb_txtable_iter_next(&it2);
    uint32_t block_start = block->m_start / FDB_TXTABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 1);
  }

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

