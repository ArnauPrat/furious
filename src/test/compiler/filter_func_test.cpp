
#include "furious.h"
#include "filter_func_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>

namespace furious
{
  

TEST(FilterFuncTest, FilterFuncTest ) 
{
  database_t database = database_create();
  furious_init(&database);

  std::vector<Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    Entity entity = FURIOUS_CREATE_ENTITY(&database);
    FURIOUS_ADD_COMPONENT(entity,Position, 0.0f, 0.0f, 0.0f);
    if(entity.m_id % 2 == 0)
    {
      FURIOUS_ADD_COMPONENT(entity,Velocity, 1.0f, 1.0f, 1.0f);
    } else
    {
      FURIOUS_ADD_COMPONENT(entity,Velocity, 2.0f, 2.0f, 2.0f);
    }
    entities.push_back(entity);
  }

  furious_frame(0.1f, &database, nullptr);

  for(Entity& entity : entities)
  {
    Position* position = FURIOUS_GET_COMPONENT(entity,Position);

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
  furious_release();
  database_destroy(&database);
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

