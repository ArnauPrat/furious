
#include "furious.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

struct Component 
{
  FURIOUS_COMPONENT(Component);

  int32_t field1_;
  float field2_;

  Component(int32_t field1, double field2) : field1_(field1), field2_(field2) {}

};

TEST(TableTest,TableWorks) 
{

  Database* database = new Database();
  FURIOUS_CREATE_TABLE(database, Component);
  //database.add_table<Component>();
  TableView<Component> table = FURIOUS_FIND_TABLE(database, Component);
  int32_t num_elements = TABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_elements; ++i) 
  {
    table.insert_element(i,i,static_cast<float>(i));
  }

  ASSERT_EQ(table.size(), num_elements);

  TableView<Component>::BlockIterator iterator = table.iterator();
  int32_t counter = 0;
  while (iterator.has_next()) 
  {
    TableView<Component>::Block block = iterator.next();
    Component* data = block.get_data();
    const Bitmap* mask = block.get_enabled();
    for (size_t i = 0; i < block.get_size(); ++i) 
    {
      ASSERT_TRUE(mask->is_set(i));
      ASSERT_EQ(data[i].field1_, counter);
      ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
      counter++;
    }
  }
  ASSERT_EQ(counter, num_elements);
  ASSERT_EQ(table.size(), num_elements);

  table.disable_element(num_elements-1);
  ASSERT_FALSE(table.is_enabled(num_elements-1));
  table.enable_element(num_elements-1);
  ASSERT_TRUE(table.is_enabled(num_elements-1));

  for(int32_t i = 0; i < num_elements; i+=2) {
    table.remove_element(i);
  }
  ASSERT_EQ(table.size(), num_elements/2);

  TableView<Component>::BlockIterator iterator2 = table.iterator();
  int32_t num_real = 0;
  counter = 0;
  while (iterator2.has_next()) {
    TableView<Component>::Block block = iterator2.next();
    const Bitmap* mask = block.get_enabled();
    Component* data = block.get_data();
    for (size_t i = 0; i < block.get_size(); ++i) {
      if(i % 2 == 0) 
      {
        ASSERT_FALSE(mask->is_set(i));
      } 
      else 
      {
        ASSERT_TRUE(mask->is_set(i));
        ASSERT_EQ(data[i].field1_, counter);
        ASSERT_EQ(data[i].field2_, static_cast<float>(counter));
        num_real++;
      }
        counter++;
    }
  }
  ASSERT_EQ(num_real, num_elements/2);

  table.clear();
  ASSERT_EQ(table.size(),0);

  //FURIOUS_REMOVE_TABLE(database, Component);
  database->remove_table<Component>();
  delete database;
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

