
#include "furious_runtime.h"
#include "filter_func_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(FilterFuncTest, FilterFuncTest ) 
{
  furious::init();
  furious::Database* database = furious::get_database();

  std::vector<furious::Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    furious::Entity entity = furious::create_entity(database);
    entity.add_component<Position>(0.0f, 0.0f, 0.0f);

    if(entity.m_id % 2 == 0)
    {
      entity.add_component<Velocity>(1.0f, 1.0f, 1.0f);
    } else
    {
      entity.add_component<Velocity>(2.0f, 2.0f, 2.0f);
    }
    entities.push_back(entity);
  }

  furious::update(0.1f);

  for(furious::Entity& entity : entities)
  {
    Position* position = entity.get_component<Position>();

    if(entity.m_id % 2 == 0)
    {
      ASSERT_EQ(position->m_x, 0.1f);
      ASSERT_EQ(position->m_y, 0.1f);
      ASSERT_EQ(position->m_z, 0.1f);
    } else
    {
      ASSERT_EQ(position->m_x, 0.0f);
      ASSERT_EQ(position->m_y, 0.0f);
      ASSERT_EQ(position->m_z, 0.0f);
    }
  }
  furious::release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

