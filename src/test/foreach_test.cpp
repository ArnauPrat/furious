
#include "../furious.h"
#include <gtest/gtest.h>

namespace furious {

struct ComponentA {
  int32_t m_field;
};

struct ComponentB {
  int32_t m_field;
};


inline void run(ComponentA* componentA, 
         const ComponentB* componentB ) {
  componentA->m_field *= componentB->m_field;
  componentA->m_field +=10;
}

TEST(ForeachTest,ForeachWorks) {

  furious::init();

  Database* database = create_database();
  database->add_table<ComponentA>();
  database->add_table<ComponentB>();

  TableView<ComponentA> tableA = database->find_table<ComponentA>();
  TableView<ComponentB> tableB = database->find_table<ComponentB>();
  int32_t num_elements = 2048*TABLE_BLOCK_SIZE;

  for(int32_t i = 0; i < num_elements; ++i) {
    tableA.insert_element(i, 5);
    tableB.insert_element(i, 5);
  }

  auto itA = tableA.iterator();
  auto itB = tableB.iterator();
  while(itA.has_next() && itB.has_next()) {
    auto blockA = itA.next();
    auto blockB = itB.next();
    block_foreach_nojoin(run, blockA, blockB);
  }

  for(int32_t i = 0; i < num_elements; ++i) {
    ASSERT_EQ(tableA.get_element(i)->m_field,35);
  }

  destroy_database(database);
  furious::release();
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}


