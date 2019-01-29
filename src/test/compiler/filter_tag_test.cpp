
#include "furious_runtime.h"
#include "filter_tag_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(FilterTagTest, FilterTagTest ) 
{
  furious::init();
  furious::Database* database = furious::get_database();

  std::vector<furious::Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    furious::Entity entity = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT((&entity), Position, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT((&entity), Velocity, 1.0f, 1.0f, 1.0f);
    if(entity.m_id % 2 == 0)
    {
      entity.add_tag("affected");
    }

    if(entity.m_id % 6 == 0)
    {
      entity.add_tag("not_affected");
    }

    entities.push_back(entity);
  }

  furious::update(0.1f);

  for(furious::Entity& entity : entities)
  {
    Position* position = FURIOUS_GET_COMPONENT((&entity),Position);

    if(entity.m_id % 2 == 0 && entity.m_id % 6 != 0)
    {
      ASSERT_EQ(position->m_x, 0.1f);
      ASSERT_EQ(position->m_y, 0.1f);
      ASSERT_EQ(position->m_z, 0.1f);
    }

    if(entity.m_id % 6 == 0 || entity.m_id % 2 != 0)
    {
      ASSERT_EQ(position->m_x , 0.0f);
      ASSERT_EQ(position->m_y , 0.0f);
      ASSERT_EQ(position->m_z , 0.0f);
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

