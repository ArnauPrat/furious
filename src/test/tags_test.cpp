
#include "../furious.h"
#include <gtest/gtest.h>

namespace furious {

struct ComponentA {
  int32_t m_field;
};

struct ComponentB {
  int32_t m_field;
};


class TestSystem {
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

TEST(TagTests,TagWorks) {

  furious::init();

  Database* database = create_database();
  database->add_table<ComponentA>();
  database->add_table<ComponentB>();

  Entity entity1 = create_entity(database);
  Entity entity2 = create_entity(database);
  Entity entity3 = create_entity(database);
  entity1.add_component<ComponentA>(1);
  entity1.add_component<ComponentB>(3);
  entity1.add_tag("selected");
  entity2.add_component<ComponentA>(2);
  entity2.add_component<ComponentB>(4);
  entity3.add_component<ComponentA>(60);
  entity3.add_tag("selected");

  Workload* workload = create_workload();
  workload->add_system<TestSystem>(10).restrict_to({"selected"});

  
  //furious::Backend* backend = new furious::Basic();
  //backend->compile(workload, database);

  //backend->run(0.0);

  ASSERT_EQ(entity1.get_component<ComponentA>()->m_field, 30 );
  ASSERT_EQ(entity2.get_component<ComponentA>()->m_field, 2 );
  ASSERT_EQ(entity3.get_component<ComponentA>()->m_field, 60 );

  destroy_entity(&entity1);
  destroy_entity(&entity2);
  destroy_entity(&entity3);
  destroy_workload(workload);
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

