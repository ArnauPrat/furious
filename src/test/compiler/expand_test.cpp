
#include "furious.h"
#include "expand_test_header.h"

#include <gtest/gtest.h>

  

TEST(ExpandTest, ExpandTestSimple)
{
  fdb_tx_init(nullptr);
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  fdb_txtable_t* sc1_table = FDB_FIND_TABLE(&database, SimpleComponent1);
  fdb_txtable_t* sc2_table = FDB_FIND_TABLE(&database, SimpleComponent2);
  fdb_txtable_t* parent_table = FDB_FIND_REF_TABLE(&database, "parent");

  entity_id_t NUM_ENTITIES = 2; 
  for(uint32_t i = 0; i < NUM_ENTITIES; ++i)
  {
    SimpleComponent1* sc1 = FDB_ADD_COMPONENT(sc1_table, &tx, txtctx, SimpleComponent1, i);
    sc1->m_x = 0.0f;
    sc1->m_y = 0.0f;
    sc1->m_z = 0.0f;

    SimpleComponent2* sc2 = FDB_ADD_COMPONENT(sc2_table, &tx, txtctx, SimpleComponent2, i);
    sc2->m_x = i;
    sc2->m_y = i;
    sc2->m_z = i;

  }

  FDB_ADD_REFERENCE(parent_table, &tx, txtctx, 0, 1);
  FDB_ADD_REFERENCE(parent_table, &tx, txtctx, 2, 3);
  FDB_ADD_REFERENCE(parent_table, &tx, txtctx, 4, 5);
  FDB_ADD_REFERENCE(parent_table, &tx, txtctx, 6, 7);
  fdb_tx_commit(&tx);

  furious_frame(0.1, &database, nullptr);

  fdb_tx_begin(&tx, E_READ_ONLY);
  txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  for(uint32_t i = 0; i < NUM_ENTITIES; i+=2)
  {
    SimpleComponent1* sc1 = FDB_GET_COMPONENT(sc1_table, &tx, txtctx, SimpleComponent1, i, false);
    float tmp = i+1;
    if(i % 2 != 0)
    {
      tmp = 0.0f;
    }
    ASSERT_EQ(sc1->m_x,tmp);
    ASSERT_EQ(sc1->m_y,tmp);
    ASSERT_EQ(sc1->m_z,tmp);
  }
  fdb_tx_commit(&tx);

  furious_release();
  fdb_database_release(&database);
  fdb_tx_release();
}

TEST(ExpandTest, ExpandTest ) 
{
  fdb_tx_init(nullptr);
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");

  furious_init(&database);

  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);

  fdb_txtable_t*      pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_txtable_t*      fm_table = FDB_FIND_TABLE(&database, FieldMesh);
  fdb_txtable_t*      vel_table = FDB_FIND_TABLE(&database, Velocity);
  fdb_txtable_t*      inty_table = FDB_FIND_TABLE(&database, Intensity);
  fdb_txtable_t*      parent_table = FDB_FIND_REF_TABLE(&database, "parent");
  fdb_txbittable_t*   test_table = FDB_FIND_TAG_TABLE(&database, &tx, txtctx, "test"); 

  entity_id_t NUM_ENTITIES = 8;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table, &tx, txtctx, Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;

    FieldMesh* fm = FDB_ADD_COMPONENT(fm_table, &tx, txtctx, FieldMesh, i);
    fm->m_x = 0.0f;
    fm->m_y = 0.0f;
    fm->m_z = 0.0f;
    
    Velocity* vel = FDB_ADD_COMPONENT(vel_table, &tx, txtctx, Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f;
    vel->m_z = 1.0f;

    Intensity* inty = FDB_ADD_COMPONENT(inty_table,  &tx, txtctx, Intensity, i);
    inty->m_intensity = 5.0f;
  }

  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 2, 0);
  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 3, 0);
  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 4, 1);
  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 5, 1);
  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 6, 3);
  FDB_ADD_REFERENCE(parent_table,  &tx, txtctx, 7, 3);
  
  FDB_ADD_TAG(test_table,  &tx, txtctx, 2);
  FDB_ADD_TAG(test_table,  &tx, txtctx, 3);
  FDB_ADD_TAG(test_table,  &tx, txtctx, 4);
  FDB_ADD_TAG(test_table,  &tx, txtctx, 5);
  fdb_tx_commit(&tx);

  furious_frame(0.1, &database, nullptr);


  fdb_tx_begin(&tx, E_READ_ONLY);
  txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  uint32_t first_level_entities[2] = {0,1};
  uint32_t second_level_entities[4] = {2,3,4,5};
  uint32_t third_level_entities[2] = {6,7};

  // FIRST LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                      &tx, 
                                      txtctx,
                                      Position, 
                                      first_level_entities[i], 
                                      false); 
    ASSERT_EQ(pos->m_x,1.0f);
    ASSERT_EQ(pos->m_y,1.0f);
    ASSERT_EQ(pos->m_z,1.0f);

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                        &tx, 
                                        txtctx,
                                        Intensity, 
                                        first_level_entities[i], 
                                        false); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                        &tx, 
                                        txtctx,
                                          FieldMesh,
                                          first_level_entities[i], 
                                          false);
    ASSERT_EQ(fm->m_x,0.0f);
    ASSERT_EQ(fm->m_y,0.0f);
    ASSERT_EQ(fm->m_z,0.0f);
  }

  // SECOND LEVEL
  for(uint32_t i = 0; i < 4; ++i)
  {

    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                        &tx, 
                                        txtctx,
                                          Position, 
                                          second_level_entities[i], 
                                          false); 
    ASSERT_EQ(pos->m_x,0.2f);
    ASSERT_EQ(pos->m_y,0.2f);
    ASSERT_EQ(pos->m_z,0.2f);

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                        &tx, 
                                        txtctx,
                                            Intensity, 
                                            second_level_entities[i], 
                                            false); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                        &tx, 
                                        txtctx,
                                          FieldMesh,
                                          second_level_entities[i], 
                                          false);
    ASSERT_EQ(fm->m_x,5.0f);
    ASSERT_EQ(fm->m_y,5.0f);
    ASSERT_EQ(fm->m_z,5.0f);
  }

  // THIRD LEVEL
  for(uint32_t i = 0; i < 2; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                        &tx, 
                                        txtctx,
                                          Position, 
                                          third_level_entities[i], 
                                          false); 
    ASSERT_TRUE(pos->m_x - 0.12f < 0.001 );
    ASSERT_TRUE(pos->m_y - 0.12f < 0.001 );
    ASSERT_TRUE(pos->m_z - 0.12f < 0.001 );

    Intensity* inty = FDB_GET_COMPONENT(inty_table,  
                                        &tx, 
                                        txtctx,
                                            Intensity, 
                                            third_level_entities[i], 
                                            false); 
    ASSERT_EQ(inty->m_intensity,5.0f);

    FieldMesh* fm = FDB_GET_COMPONENT(fm_table, 
                                        &tx, 
                                        txtctx,
                                          FieldMesh,
                                          third_level_entities[i], 
                                          false);
    ASSERT_EQ(fm->m_x,2.0f);
    ASSERT_EQ(fm->m_y,2.0f);
    ASSERT_EQ(fm->m_z,2.0f);
  }
  fdb_tx_commit(&tx);

  furious_release();
  fdb_database_release(&database);
  fdb_tx_release();
}

TEST(ExpandTest, ExpandTestLarge ) 
{
  fdb_tx_init(nullptr); 
  fdb_database_t database;
  fdb_database_init(&database, nullptr);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  struct fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  fdb_txtable_t*      pos_table = FDB_FIND_TABLE(&database, Position);
  fdb_txtable_t*      fm_table = FDB_FIND_TABLE(&database, FieldMesh);
  fdb_txtable_t*      vel_table = FDB_FIND_TABLE(&database, Velocity);
  fdb_txtable_t*      inty_table = FDB_FIND_TABLE(&database, Intensity);
  fdb_txtable_t*  parent_table = FDB_FIND_REF_TABLE(&database, "parent");

  const entity_id_t NUM_ENTITIES = 65536;
  for(entity_id_t i = 0; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_ADD_COMPONENT(pos_table,&tx, txtctx, Position, i);
    pos->m_x = 0.0f;
    pos->m_y = 0.0f;
    pos->m_z = 0.0f;

    FieldMesh* fm = FDB_ADD_COMPONENT(fm_table,&tx, txtctx, FieldMesh, i);
    fm->m_x = 0.0f;
    fm->m_y = 0.0f;
    fm->m_z = 0.0f;
    
    Velocity* vel = FDB_ADD_COMPONENT(vel_table,&tx, txtctx, Velocity, i);
    vel->m_x = 1.0f;
    vel->m_y = 1.0f;
    vel->m_z = 1.0f;

    Intensity* inty = FDB_ADD_COMPONENT(inty_table, &tx, txtctx, Intensity, i);
    inty->m_intensity = 5.0f;
  }

  for(entity_id_t i = 1; i < NUM_ENTITIES; ++i)
  {
    FDB_ADD_REFERENCE(parent_table, &tx, txtctx, i, 0);
  }
  fdb_tx_commit(&tx);

  furious_frame(0.1, &database, nullptr);

  fdb_tx_begin(&tx, E_READ_ONLY);
  txtctx = fdb_tx_txthread_ctx_get(&tx, nullptr);
  for(entity_id_t i = 1; i < NUM_ENTITIES; ++i)
  {
    Position* pos = FDB_GET_COMPONENT(pos_table, 
                                      &tx, 
                                      txtctx,
                                      Position, 
                                      i, 
                                      false); 
    ASSERT_EQ(pos->m_x, 0.2f);
    ASSERT_EQ(pos->m_y, 0.2f);
    ASSERT_EQ(pos->m_z, 0.2f);
  }
  fdb_tx_commit(&tx);

  furious_release();
  fdb_database_release(&database);
  fdb_tx_release();
}


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

