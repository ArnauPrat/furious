
#include "furious.h"
#include "expand_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(ExpandTest, ExpandTestSimple)
{

  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");
  furious::__furious_init(database);

  furious::Entity entities[8];
  for(uint32_t i = 0; i < 8; ++i)
  {
    entities[i] = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT(entities[i],SimpleComponent1, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(entities[i],SimpleComponent2, (float)i, (float)i, (float)i);
  }

  entities[0].add_reference("parent",entities[1]);
  entities[2].add_reference("parent",entities[3]);
  entities[4].add_reference("parent",entities[5]);
  entities[6].add_reference("parent",entities[7]);

  furious::__furious_frame(0.1,database, nullptr);

  for(uint32_t i = 0; i < 8; i+=2)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_x,(float)(i+1));
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_y,(float)(i+1));
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_z,(float)(i+1));
    }
    else
    {
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_x,0.0f);
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_y,0.0f);
      ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], SimpleComponent1)->m_z,0.0f);
    }
  }

  furious::__furious_release();

  delete database;
}

TEST(ExpandTest, ExpandTest ) 
{
  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");

  furious::__furious_init(database);

  furious::Entity entities[8];
  for(uint32_t i = 0; i < 8; ++i)
  {
    entities[i] = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT(entities[i],Position, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(entities[i],FieldMesh, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(entities[i],Velocity, 1.0f, 1.0f, 1.0f);
    FURIOUS_ADD_COMPONENT(entities[i],Intensity, 5.0f);
  }

  entities[2].add_reference("parent",entities[0]);
  entities[3].add_reference("parent",entities[0]);
  entities[4].add_reference("parent",entities[1]);
  entities[5].add_reference("parent",entities[1]);
  entities[6].add_reference("parent",entities[3]);
  entities[7].add_reference("parent",entities[3]);

  
  entities[2].add_tag("test");
  entities[3].add_tag("test");
  entities[4].add_tag("test");
  entities[5].add_tag("test");

  furious::__furious_frame(0.1,database, nullptr);

  uint32_t first_level_entities[2] = {0,1};
  uint32_t second_level_entities[4] = {2,3,4,5};
  uint32_t third_level_entities[2] = {6,7};

  // FIRST LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], Position)->m_x,1.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], Position)->m_y,1.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], Position)->m_z,1.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], FieldMesh)->m_x,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], FieldMesh)->m_y,0.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[first_level_entities[i]], FieldMesh)->m_z,0.0f);
  }

  // SECOND LEVEL
  for(uint32_t i = 0; i < 4; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], Position)->m_x,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], Position)->m_y,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], Position)->m_z,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], FieldMesh)->m_x,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], FieldMesh)->m_y,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[second_level_entities[i]], FieldMesh)->m_z,5.0f);
  }

  // THIRD LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    ASSERT_TRUE(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], Position)->m_x - 0.12f < 0.001);
    ASSERT_TRUE(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], Position)->m_y - 0.12f < 0.001);
    ASSERT_TRUE(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], Position)->m_z - 0.12f < 0.001);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], Intensity)->m_intensity,5.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], FieldMesh)->m_x,2.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], FieldMesh)->m_y,2.0f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[third_level_entities[i]], FieldMesh)->m_z,2.0f);
  }

  furious::__furious_release();

  delete database;
}

TEST(ExpandTest, ExpandTestLarge ) 
{
  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");
  furious::__furious_init(database);

  constexpr uint32_t num_entities = 65536;
  furious::Entity entities[num_entities];
  for(uint32_t i = 0; i < num_entities; ++i)
  {
    entities[i] = furious::create_entity(database);
    FURIOUS_ADD_COMPONENT(entities[i],Position, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(entities[i],Velocity, 1.0f, 1.0f, 1.0f);
    FURIOUS_ADD_COMPONENT(entities[i],FieldMesh, 0.0f, 0.0f, 0.0f);
    FURIOUS_ADD_COMPONENT(entities[i],Intensity, 5.0f);
  }

  for(uint32_t i = 1; i < num_entities; ++i)
  {
    entities[i].add_reference("parent",entities[0]);
  }

  furious::__furious_frame(0.1,database, nullptr);

  for(uint32_t i = 1; i < num_entities; ++i)
  {
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], Position)->m_x,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], Position)->m_y,0.2f);
    ASSERT_EQ(FURIOUS_GET_COMPONENT(entities[i], Position)->m_z,0.2f);
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

