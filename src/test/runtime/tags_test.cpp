
#include "furious.h"
#include <gtest/gtest.h>

namespace furious {

struct ComponentA 
{
  FURIOUS_COMPONENT(ComponentA);

  int32_t m_field;

  static const char* name()
  {
    return "ComponentA";
  }
};

struct ComponentB 
{
  FURIOUS_COMPONENT(ComponentB);

  int32_t m_field;

  static const char* name()
  {
    return "ComponentB";
  }
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

TEST(TagTests,TagWorks) 
{
  Database* database = new Database();
  FURIOUS_CREATE_TABLE(database, ComponentA);
  FURIOUS_CREATE_TABLE(database, ComponentB);

  Entity entity1 = create_entity(database);
  Entity entity2 = create_entity(database);
  Entity entity3 = create_entity(database);
  entity1.add_tag("selected");
  entity3.add_tag("selected");

  ASSERT_TRUE(entity1.has_tag("selected"));
  ASSERT_FALSE(entity2.has_tag("selected"));
  ASSERT_TRUE(entity3.has_tag("selected"));

  destroy_entity(entity1);
  destroy_entity(entity2);
  destroy_entity(entity3);
  delete database;
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

