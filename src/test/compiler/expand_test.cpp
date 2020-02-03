
#include "furious.h"
#include "expand_test_header.h"

#include <gtest/gtest.h>

  

TEST(ExpandTest, ExpandTestSimple)
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  fdb_table_t* sc1_table = FDB_FIND_TABLE(&database, SimpleComponent1);
  fdb_table_t* sc2_table = FDB_FIND_TABLE(&database, SimpleComponent2);
  fdb_reftable_t* parent_table = FDB_FIND_REF_TABLE(&database, "parent");

  entity_id_t NUM_ENTITIES = 8; 
  for(uint32_t i = 0; i < 8; ++i)
  {
    SimpleComponent1* sc1 = FDB_ADD_COMPONENT(sc1_table, SimpleComponent1, i);
    sc1->m_x = 0.0f;
    sc1->m_y = 0.0f;
    sc1->m_z = 0.0f;

    SimpleComponent2* sc2 = FDB_ADD_COMPONENT(sc2_table, SimpleComponent2, i);
    sc2->m_x = i;
    sc2->m_y = i;
    sc2->m_z = i;

  }

  FDB_ADD_REFERENCE(parent_table, 0, 1);
  FDB_ADD_REFERENCE(parent_table, 2, 3);
  FDB_ADD_REFERENCE(parent_table, 4, 5);
  FDB_ADD_REFERENCE(parent_table, 6, 7);

  furious_frame(0.1, &database, nullptr);

  for(uint32_t i = 0; i < NUM_ENTITIES; i+=2)
  {
    SimpleComponent1* sc1 = FDB_GET_COMPONENT(sc1_table, SimpleComponent1, i);
    float tmp = i+1;
    if(i % 2 != 0)
    {
      tmp = 0.0f;
    }
    ASSERT_EQ(sc1->m_x,tmp);
    ASSERT_EQ(sc1->m_y,tmp);
    ASSERT_EQ(sc1->m_z,tmp);
  }

  furious_release();
  fdb_database_release(&database);
}

TEST(ExpandTest, ExpandTest ) 
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");

  furious_init(&database);

  fdb_table_t*      pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_table_t*      fm_table = FDB_FIND_TABLE(&database, FieldMesh);
  fdb_table_t*      vel_table = FDB_FIND_TABLE(&database, Velocity);
  fdb_table_t*      inty_table = FDB_FIND_TABLE(&database, Intensity);
  fdb_reftable_t*       parent_table = FDB_FIND_REF_TABLE(&database, "parent");
  fdb_bittable_t*   test_table = FDB_FIND_TAG_TABLE(&database, "test"); 

  entity_id_t NUM_ENTITIES = 8;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table,Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;

    FieldMesh* fm = FDB_ADD_COMPONENT(fm_table,FieldMesh, i);
    fm->m_x = 0.0f;
    fm->m_y = 0.0f;
    fm->m_z = 0.0f;
    
    Velocity* vel = FDB_ADD_COMPONENT(vel_table,Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f;
    vel->m_z = 1.0f;

    Intensity* inty = FDB_ADD_COMPONENT(inty_table, Intensity, i);
    inty->m_intensity = 5.0f;
  }

  FDB_ADD_REFERENCE(parent_table, 2, 0);
  FDB_ADD_REFERENCE(parent_table, 3, 0);
  FDB_ADD_REFERENCE(parent_table, 4, 1);
  FDB_ADD_REFERENCE(parent_table, 5, 1);
  FDB_ADD_REFERENCE(parent_table, 6, 3);
  FDB_ADD_REFERENCE(parent_table, 7, 3);
  
  FDB_ADD_TAG(test_table, 2);
  FDB_ADD_TAG(test_table, 3);
  FDB_ADD_TAG(test_table, 4);
  FDB_ADD_TAG(test_table, 5);

  furious_frame(0.1, &database, nullptr);

  uint32_t first_level_entities[2] = {0,1};
  uint32_t second_level_entities[4] = {2,3,4,5};
  uint32_t third_level_entities[2] = {6,7};

  // FIRST LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                          Position, 
                                          first_level_entities[i]); 
    ASSERT_EQ(pos->m_x,1.0f);
    ASSERT_EQ(pos->m_y,1.0f);
    ASSERT_EQ(pos->m_z,1.0f);

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                            Intensity, 
                                            first_level_entities[i]); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                          FieldMesh,
                                          first_level_entities[i]);
    ASSERT_EQ(fm->m_x,0.0f);
    ASSERT_EQ(fm->m_y,0.0f);
    ASSERT_EQ(fm->m_z,0.0f);
  }

  // SECOND LEVEL
  for(uint32_t i = 0; i < 4; ++i)
  {

    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                          Position, 
                                          second_level_entities[i]); 
    ASSERT_EQ(pos->m_x,0.2f);
    ASSERT_EQ(pos->m_y,0.2f);
    ASSERT_EQ(pos->m_z,0.2f);

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                            Intensity, 
                                            second_level_entities[i]); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                          FieldMesh,
                                          second_level_entities[i]);
    ASSERT_EQ(fm->m_x,5.0f);
    ASSERT_EQ(fm->m_y,5.0f);
    ASSERT_EQ(fm->m_z,5.0f);
  }

  // THIRD LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                          Position, 
                                          third_level_entities[i]); 
    ASSERT_TRUE(pos->m_x - 0.12f < 0.001 );
    ASSERT_TRUE(pos->m_y - 0.12f < 0.001 );
    ASSERT_TRUE(pos->m_z - 0.12f < 0.001 );

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                            Intensity, 
                                            third_level_entities[i]); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                          FieldMesh,
                                          third_level_entities[i]);
    ASSERT_EQ(fm->m_x,2.0f);
    ASSERT_EQ(fm->m_y,2.0f);
    ASSERT_EQ(fm->m_z,2.0f);
  }

  furious_release();
  fdb_database_release(&database);
}

TEST(ExpandTest, ExpandTestLarge ) 
{
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  fdb_table_t*      pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_table_t*      fm_table = FDB_FIND_TABLE(&database, FieldMesh);
  fdb_table_t*      vel_table = FDB_FIND_TABLE(&database, Velocity);
  fdb_table_t*      inty_table = FDB_FIND_TABLE(&database, Intensity);
  fdb_reftable_t*  parent_table = FDB_FIND_REF_TABLE(&database, "parent");

  const entity_id_t NUM_ENTITIES = 65536;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table,Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;

    FieldMesh* fm = FDB_ADD_COMPONENT(fm_table,FieldMesh, i);
    fm->m_x = 0.0f;
    fm->m_y = 0.0f;
    fm->m_z = 0.0f;
    
    Velocity* vel = FDB_ADD_COMPONENT(vel_table,Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f;
    vel->m_z = 1.0f;

    Intensity* inty = FDB_ADD_COMPONENT(inty_table, Intensity, i);
    inty->m_intensity = 5.0f;
  }

  for(entity_id_t i = 1; i < NUM_ENTITIES; ++i)
  {
  FDB_ADD_REFERENCE(parent_table, i, 0);
  }

  furious_frame(0.1, &database, nullptr);

  for(entity_id_t i = 1; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                          Position, 
                                          i); 
    ASSERT_EQ(pos->m_x, 0.2f);
    ASSERT_EQ(pos->m_y, 0.2f);
    ASSERT_EQ(pos->m_z, 0.2f);
  }

  furious_release();
  fdb_database_release(&database);
}


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

