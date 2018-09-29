
#include "../runtime/system_wrapper.h"
#include "../furious.h"

#include <gtest/gtest.h>

namespace furious 
{

struct ComponentA 
{
  int32_t m_field;
};

struct ComponentB 
{
  int32_t m_field;
};


class TestSystem 
{
public:
  TestSystem(uint32_t val) : m_val{val} {}
  virtual ~TestSystem() = default;

  void run(Context* context, 
           int32_t id, 
           ComponentA* componentA, 
           const ComponentB* componentB ) {
    componentA->m_field = componentB->m_field*m_val;
  }

  uint32_t m_val;
};

TEST(SystemTest, SystemWorks) 
{
  /*
  init();
  Database* database = create_database();
  database->add_table<ComponentA>();
  database->add_table<ComponentB>();
  TableView<ComponentA> tableA = database->find_table<ComponentA>();
  TableView<ComponentB> tableB = database->find_table<ComponentB>();

  int32_t num_elements = TABLE_BLOCK_SIZE * 10;
  for(int32_t i = 0; i < num_elements; ++i) {
    tableA.insert_element(i, i);
    tableB.insert_element(i, i*2);
  }

  auto test_system = create_static_system<TestSystem>(0,5);

  // Checking if we correctly extract the const modifier from the types. 
  ASSERT_EQ(test_system->components()[0].m_access_type, ComAccessType::E_WRITE );
  ASSERT_EQ(test_system->components()[1].m_access_type, ComAccessType::E_READ );

  auto it1 = tableA.iterator();
  auto it2 = tableB.iterator();
  while(it1.has_next() && it2.has_next()) {
    TBlock* blockA = it1.next();
    TBlock* blockB = it2.next();
    std::vector<TBlock*> blocks{blockA, blockB};
    test_system->apply_block(nullptr,blocks);
  }

  auto itA = tableA.iterator();
  auto itB = tableB.iterator();
  while(itA.has_next() && itB.has_next()) {
    TableView<ComponentA>::Row next_rowA = itA.next();
    TableView<ComponentB>::Row next_rowB = itB.next();
    ASSERT_EQ(next_rowA.p_component->m_field, next_rowB.p_component->m_field*5);
  }

  destroy_database(database);
  release();
  */
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
