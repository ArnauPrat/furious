
#include "furious.h"
#include <gtest/gtest.h>

namespace furious {

FURIOUS_BEGIN_COMPONENT(ComponentA, KILOBYTES(4))
  uint32_t m_field;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(ComponentB, KILOBYTES(4))
  uint32_t m_field;
FURIOUS_END_COMPONENT

TEST(RefsTest,TagWorks) 
{
  database_t database = database_create();
  FURIOUS_CREATE_TABLE(&database, ComponentA);
  FURIOUS_CREATE_TABLE(&database, ComponentB);

  Entity entity1 = create_entity(&database);
  Entity entity2 = create_entity(&database);
  Entity entity3 = create_entity(&database);


  entity1.add_reference("test_ref", entity2);
  entity2.add_reference("test_ref", entity3);

  ASSERT_TRUE(entity1.get_reference("test_ref").m_id == entity2.m_id);
  ASSERT_TRUE(entity2.get_reference("test_ref").m_id == entity3.m_id);

  entity1.add_reference("test_ref", entity3);

  ASSERT_TRUE(entity1.get_reference("test_ref").m_id == entity3.m_id);

  entity1.remove_reference("test_ref");

  ASSERT_FALSE(entity1.get_reference("test_ref").is_valid());

  destroy_entity(entity1);
  destroy_entity(entity2);
  destroy_entity(entity3);
  database_destroy(&database);
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

