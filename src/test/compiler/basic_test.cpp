
#include "furious_runtime.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(BasicTest, BasicTest ) 
{
  furious::init();
  furious::Database* database = furious::get_database();

  std::vector<furious::Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    furious::Entity entity = furious::create_entity(database);
    entity.add_component<Position>(0.0f, 0.0f, 0.0f);
    entity.add_component<Velocity>(1.0f, 1.0f, 1.0f);
    entities.push_back(entity);
  }

  furious::update(0.1);

  for(furious::Entity& entity : entities)
  {
    Position* position = entity.get_component<Position>();
    ASSERT_TRUE(position->m_x == 0.1f);
    ASSERT_TRUE(position->m_y == 0.1f);
    ASSERT_TRUE(position->m_z == 0.1f);
  }
  furious::release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
