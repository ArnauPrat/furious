
#include "furious.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

FURIOUS_BEGIN_COMPONENT(Component, KILOBYTES(4))

  int32_t field1_;
  float field2_;

  Component(int32_t field1, double field2) : field1_(field1), field2_(field2) {}

FURIOUS_END_COMPONENT

TEST(TableTest,TableWorks) 
{

  database_t database = database_create();
  FURIOUS_CREATE_TABLE(&database, Component);
  //database.add_table<Component>();
  TableView<Component> table = FURIOUS_FIND_TABLE(&database, Component);
  int32_t num_components = FURIOUS_TABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_components; ++i) 
  {
    table.insert_component(i,i,static_cast<float>(i));
  }

  ASSERT_EQ(table.size(), num_components);

  TableView<Component>::BlockIterator iterator = table.iterator();
  int32_t counter = 0;
  while (iterator.has_next()) 
  {
    TableView<Component>::Block block = iterator.next();
    Component* data = block.get_data();
    const bitmap_t* mask = block.get_enabled();
    for (size_t i = 0; i < block.get_size(); ++i) 
    {
      ASSERT_TRUE(bitmap_is_set(mask, i));
      ASSERT_EQ(data[i].field1_, counter);
      ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
      counter++;
    }
  }
  ASSERT_EQ(counter, num_components);
  ASSERT_EQ(table.size(), num_components);

  table.disable_component(num_components-1);
  ASSERT_FALSE(table.is_enabled(num_components-1));
  table.enable_component(num_components-1);
  ASSERT_TRUE(table.is_enabled(num_components-1));

  for(int32_t i = 0; i < num_components; i+=2) {
    table.remove_component(i);
  }
  ASSERT_EQ(table.size(), num_components/2);

  TableView<Component>::BlockIterator iterator2 = table.iterator();
  int32_t num_real = 0;
  counter = 0;
  while (iterator2.has_next()) {
    TableView<Component>::Block block = iterator2.next();
    const bitmap_t* mask = block.get_enabled();
    Component* data = block.get_data();
    for (size_t i = 0; i < block.get_size(); ++i) {
      if(i % 2 == 0) 
      {
        ASSERT_FALSE(bitmap_is_set(mask, i));
      } 
      else 
      {
        ASSERT_TRUE(bitmap_is_set(mask, i));
        ASSERT_EQ(data[i].field1_, counter);
        ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
        num_real++;
      }
        counter++;
    }
  }
  ASSERT_EQ(num_real, num_components/2);

  table.clear();
  ASSERT_EQ(table.size(),0);

  FURIOUS_REMOVE_TABLE(&database, Component);
  database_destroy(&database);
}


TEST(IteratorTest,TableWorks) 
{

  database_t database = database_create();
  FURIOUS_CREATE_TABLE(&database, Component);
  TableView<Component> table = FURIOUS_FIND_TABLE(&database, Component);
  int32_t num_components = FURIOUS_TABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_components; ++i) 
  {
    table.insert_component(i,i,static_cast<float>(i));
  }

  TableView<Component>::BlockIterator it = table.iterator(1, 0, 2);
  while(it.has_next())
  {
    TableView<Component>::Block block = it.next();
    uint32_t block_start = block.get_start() / FURIOUS_TABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 0);
  }

  TableView<Component>::BlockIterator it2 = table.iterator(1, 1, 2);
  while(it2.has_next())
  {
    TableView<Component>::Block block = it2.next();
    uint32_t block_start = block.get_start() / FURIOUS_TABLE_BLOCK_SIZE;
    ASSERT_TRUE(block_start % 2 == 1);
  }

  database_destroy(&database);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

