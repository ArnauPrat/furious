
#include "furious.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>

namespace furious
{
  

TEST(BasicTest, BasicTest ) 
{
  database_t database = database_create();
  database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious::furious_init(&database);

  std::vector<furious::Entity> entities;
  for(int i = 0; i < 1000; ++i)
  {
    furious::Entity entity = furious::create_entity(&database);
    FURIOUS_ADD_COMPONENT(entity,Position,0.0f,0.0f,0.0f);
    FURIOUS_ADD_COMPONENT(entity,Velocity,1.0f,1.0f,1.0f);
    entities.push_back(entity);
  }

  furious::furious_frame(0.1,&database, nullptr);

  for(furious::Entity& entity : entities)
  {
    Position* position = FURIOUS_GET_COMPONENT(entity, Position);
    ASSERT_EQ(position->m_x,-0.1f);
    ASSERT_EQ(position->m_y,-0.1f);
    ASSERT_EQ(position->m_z,-0.1f);
  }
  furious::furious_release();
  database_destroy(&database);
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

