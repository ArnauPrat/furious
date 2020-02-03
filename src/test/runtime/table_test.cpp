
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

  fdb_database_t database;
  fdb_database_init(&database, 
                    nullptr);
  fdb_table_t* t = FDB_CREATE_TABLE(&database, Component, nullptr);
  entity_id_t num_entities = FDB_TABLE_BLOCK_SIZE*2048;

  for(entity_id_t i = 0; i < num_entities; ++i) 
  {
    Component* c = FDB_ADD_COMPONENT(t, Component, i);
    c->field1_ = i;
    c->field2_ = i;
  }

  ASSERT_EQ(fdb_table_size(t), num_entities);

  fdb_table_iter_t iter;
  fdb_table_iter_init(&iter, t, 1, 0, 1);
  int32_t counter = 0;
  while (fdb_table_iter_has_next(&iter)) 
  {
    fdb_table_block_t* block = fdb_table_iter_next(&iter);
    Component* data = (Component*)block->p_data;
    const fdb_bitmap_t* mask = &block->m_enabled;
    for (size_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i) 
    {
      ASSERT_TRUE(fdb_bitmap_is_set(mask, i));
      ASSERT_EQ(data[i].field1_, counter);
      ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
      counter++;
    }
  }
  fdb_table_iter_release(&iter);
  ASSERT_EQ(counter, num_entities);
  ASSERT_EQ(fdb_table_size(t), num_entities);

  fdb_table_disable_component(t, num_entities-1);
  ASSERT_FALSE(fdb_table_is_enabled(t, num_entities-1));
  fdb_table_enable_component(t, num_entities-1);
  ASSERT_TRUE(fdb_table_is_enabled(t, num_entities-1));

  for(entity_id_t i = 0; i < num_entities; i+=2) {
    FDB_REMOVE_COMPONENT(t, i);
  }
  ASSERT_EQ(fdb_table_size(t), num_entities/2);

  fdb_table_iter_t iter2;
  fdb_table_iter_init(&iter2, t, 1, 0, 1);
  int32_t num_real = 0;
  counter = 0;
  while (fdb_table_iter_has_next(&iter2)) 
  {
    fdb_table_block_t* block = fdb_table_iter_next(&iter2);
    const fdb_bitmap_t* mask = &block->m_enabled;
    Component* data = (Component*)block->p_data;
    for (size_t i = 0; i < FDB_TABLE_BLOCK_SIZE; ++i) 
    {
      if(i % 2 == 0) 
      {
        ASSERT_FALSE(fdb_bitmap_is_set(mask, i));
      } 
      else 
      {
        ASSERT_TRUE(fdb_bitmap_is_set(mask, i));
        ASSERT_EQ(data[i].field1_, counter);
        ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
        num_real++;
      }
        counter++;
    }
  }
  fdb_table_iter_release(&iter2);
  ASSERT_EQ(num_real, num_entities/2);

  fdb_table_clear(t);
  ASSERT_EQ(fdb_table_size(t),0);

  FDB_REMOVE_TABLE(&database, Component);
  fdb_database_release(&database);
}


TEST(IteratorTest,TableWorks) 
{

  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_table_t* t = FDB_CREATE_TABLE(&database, Component, nullptr);
  int32_t num_entities = FDB_TABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_entities; ++i) 
  {
    Component* c = FDB_ADD_COMPONENT(t, Component, i);
    c->field1_ = i;
    c->field2_ = i;
  }

  fdb_table_iter_t it;
  fdb_table_iter_init(&it, 
                      t, 1, 0, 2);
  while(fdb_table_iter_has_next(&it))
  {
    fdb_table_block_t* block = fdb_table_iter_next(&it);
    uint32_t block_start = block->m_start / FDB_TABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 0);
  }
  fdb_table_iter_release(&it);

  fdb_table_iter_t it2;
  fdb_table_iter_init(&it2, t, 1, 1, 2);
  while(fdb_table_iter_has_next(&it2))
  {
    fdb_table_block_t* block = fdb_table_iter_next(&it2);
    uint32_t block_start = block->m_start / FDB_TABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 1);
  }

  fdb_database_release(&database);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

