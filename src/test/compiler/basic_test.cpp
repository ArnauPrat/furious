
#include "furious.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(BasicTest, BasicTest ) 
{
  furious::Database* database = new furious::Database();
  furious::__furious_init(database);

  std::vector<furious::Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    furious::Entity entity = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT((&entity),Position,0.0f,0.0f,0.0f);
    FURIOUS_ADD_COMPONENT((&entity),Velocity,1.0f,1.0f,1.0f);
    entities.push_back(entity);
  }

  furious::__furious_frame(0.1,database);

  for(furious::Entity& entity : entities)
  {
    Position* position = FURIOUS_GET_COMPONENT((&entity), Position);
    ASSERT_TRUE(position->m_x == 0.1f);
    ASSERT_TRUE(position->m_y == 0.1f);
    ASSERT_TRUE(position->m_z == 0.1f);
  }
  furious::__furious_release();
  delete database;
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

