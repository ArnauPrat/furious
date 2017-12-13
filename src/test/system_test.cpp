
#include "../runtime/static_system.h"
#include "../furious.h"

#include <gtest/gtest.h>

namespace furious {

struct ComponentA {
  uint32_t m_field;
};

struct ComponentB {
  uint32_t m_field;
};


class TestSystem {
public:
  TestSystem(uint32_t val) : m_val{val} {}
  virtual ~TestSystem() = default;

  void run(ComponentA* componentA, const ComponentB* componentB ) {
    componentA->m_field = componentB->m_field*m_val;
  }

  uint32_t m_val;
};

TEST(SystemTest, SystemWorks) {
 
  init();
  register_component<ComponentA>();
  register_component<ComponentB>();
  Database* database = Database::get_instance();
  Table* tableA = database->find_table<ComponentA>();
  Table* tableB = database->find_table<ComponentB>();

  uint32_t num_elements = TABLE_BLOCK_SIZE * 10;
  for(uint32_t i = 0; i < num_elements; ++i) {
    tableA->insert_element<ComponentA>(i, i);
    tableB->insert_element<ComponentB>(i, i*2);
  }

  auto test_system = create_static_system<TestSystem>(5);

  // Checking if we correctly extract the const modifier from the types. 
  ASSERT_EQ(test_system->components()[0].m_access_type, ComAccessType::E_WRITE );
  ASSERT_EQ(test_system->components()[1].m_access_type, ComAccessType::E_READ );

  Table::Iterator* itA = tableA->iterator();
  Table::Iterator* itB = tableB->iterator();
  while(itA->has_next() && itB->has_next()) {
    TBlock* next_blockA = itA->next();
    TBlock* next_blockB = itB->next();
    std::vector<void*> blocks{next_blockA->p_data, next_blockB->p_data};
    test_system->apply_block(blocks);
  }
  delete itA;
  delete itB;

  uint32_t counter = 0;
  itA = tableA->iterator();
  while(itA->has_next()) {
    TBlock* next_block = itA->next();
    ComponentA* data = reinterpret_cast<ComponentA*>(next_block->p_data);
    for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i, ++counter) {
      ComponentA* component = &data[i];
      ASSERT_EQ(component->m_field, counter*2*5);
    }
  }
  delete itA;

  release();
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
