
#include "../furious.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

struct Component {
  int32_t field1_;
  float field2_;

  Component(int32_t field1, double field2) : field1_(field1), field2_(field2) {}

};

TEST(TableTest,TableWorks) {

  init();
  Database* database = create_database();
  database->add_table<Component>();
  TableView<Component> table = database->find_table<Component>();
  int32_t num_elements = TABLE_BLOCK_SIZE*2048;

  for(int32_t i = 0; i < num_elements; ++i) {
    table.insert_element(i,i,static_cast<float>(i));
  }

  ASSERT_EQ(table.size(), num_elements);

  TableView<Component>::Iterator iterator = table.iterator();
  int32_t counter = 0;
  while (iterator.has_next()) {
    TableView<Component>::Row row = iterator.next();
    ASSERT_EQ(row.p_component->field1_, row.m_id);
    ASSERT_EQ(row.p_component->field2_, static_cast<float>(row.m_id));
    counter++;
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

  TableView<Component>::Iterator iterator2 = table.iterator();
  counter = 0;
  while (iterator2.has_next()) {
    TableView<Component>::Row row = iterator2.next();
    ASSERT_EQ(row.p_component->field1_, row.m_id);
    ASSERT_EQ(row.p_component->field2_, static_cast<float>(row.m_id));
    counter++;
  }
  ASSERT_EQ(counter, num_elements/2);

  table.clear();
  ASSERT_EQ(table.size(),0);

  database->remove_table<Component>();
  destroy_database(database);
  release();
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

