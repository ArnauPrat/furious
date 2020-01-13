
#include "furious.h"
#include "global_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>

namespace furious
{
  
TEST(GlobalTest, GlobalTest ) 
{
  database_t database = database_create();
  database_start_webserver(&database, 
                           "localhost", 
                           "8080");

  FURIOUS_CREATE_GLOBAL(&database, GlobalComponent, 1.0f, 1.0f, 1.0f);

  Entity entities[8];
  for(uint32_t i = 0; i < 8; ++i)
  {
    entities[i] = FURIOUS_CREATE_ENTITY(&database);
    FURIOUS_ADD_COMPONENT(entities[i],Position, 0.0f, 0.0f, 0.0f);
  }

  furious_init(&database);
  furious_frame(0.1, &database, nullptr);

  // FIRST LEVEL
  for(uint32_t i = 0; i < 8; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i],Position)->m_x, 1.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i],Position)->m_y, 1.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i],Position)->m_z, 1.0f);
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

