
#include "furious.h"
#include "expand_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(ExpandTest, ExpandTest ) 
{
  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");

  furious::Entity entities[8];
  for(uint32_t i = 0; i < 8; ++i)
  {
    entities[i] = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT(&entities[i],Position, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(&entities[i],Velocity, 1.0f, 1.0f, 1.0f);
    FURIOUS_ADD_COMPONENT(&entities[i],FieldMesh, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(&entities[i],Intensity, 5.0f);
  }

  entities[2].add_reference("parent",entities[0]);
  entities[3].add_reference("parent",entities[0]);
  entities[6].add_reference("parent",entities[3]);
  entities[7].add_reference("parent",entities[3]);

  entities[4].add_reference("parent",entities[1]);
  entities[5].add_reference("parent",entities[1]);
  


  furious::__furious_init(database);
  furious::__furious_frame(0.1,database);

  uint32_t first_level_entities[2] = {0,1};
  uint32_t second_level_entities[4] = {2,3,4,5};
  uint32_t third_level_entities[2] = {6,7};

  // FIRST LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], Position)->m_x,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], Position)->m_y,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], Position)->m_z,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], FieldMesh)->m_x,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], FieldMesh)->m_y,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[first_level_entities[i]], FieldMesh)->m_z,0.0f);
  }

  // SECOND LEVEL
  for(uint32_t i = 0; i < 4; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], Position)->m_x,0.1f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], Position)->m_y,0.1f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], Position)->m_z,0.1f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], FieldMesh)->m_x,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], FieldMesh)->m_y,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[second_level_entities[i]], FieldMesh)->m_z,0.0f);
  }

  // THIRD LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], Position)->m_x,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], Position)->m_y,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], Position)->m_z,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], FieldMesh)->m_x,0.5f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], FieldMesh)->m_y,0.5f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(&entities[third_level_entities[i]], FieldMesh)->m_z,0.5f);
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

