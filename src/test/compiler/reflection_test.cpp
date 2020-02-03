
#include "furious.h"
#include "reflection_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


  
TEST(ReflectionTest, ReflectionTest ) 
{
  fdb_database_t database;
  fdb_database_init(&database, NULL);
  fdb_database_start_webserver(&database, 
                           "localhost", 
                           "8080");
  furious_init(&database);

  fdb_table_t* tc_table = FDB_FIND_TABLE(&database, TestComponent);
  entity_id_t ent = 0;
  TestComponent* tc = FDB_ADD_COMPONENT(tc_table, TestComponent, ent);
  tc->m_x = 1.0f;
  tc->m_y = 2.0f; 
  tc->m_z = 3.0f;
  tc->m_q = 4.0f;
  tc->m_t = 5.0f;
  tc->X.m_a = 6.0f;
  tc->X.m_b = 6.0f;

  furious_frame(0.1, &database, nullptr);

  void* raw_pointer = tc;
  const fdb_mstruct_t* mstruct = FDB_GET_REFL_DATA(&database, TestComponent);
  ASSERT_TRUE(mstruct != nullptr);
  for(uint32_t i = 0; i < mstruct->m_nfields; ++i)
  {
    const fdb_mfield_t* field = mstruct->p_fields[i];
    if(strcmp(field->m_name,"m_x")==0)
    {
      float* field_ptr = (float*)((char*)raw_pointer + field->m_offset);
      ASSERT_EQ(*field_ptr,1.0f);
      continue;
    }

    if(strcmp(field->m_name, "m_y")==0)
    {
      printf("%ld\n", field->m_offset);
      float* field_ptr = (float*)((char*)raw_pointer + field->m_offset);
      ASSERT_EQ(*field_ptr,2.0f);
      continue;
    }

    if(strcmp(field->m_name, "m_z")==0)
    {
      float* field_ptr = (float*)((char*)raw_pointer + field->m_offset);
      ASSERT_EQ(*field_ptr , 3.0f);
      continue;
    }

    if(strcmp(field->m_name,"") == 0 && field->m_anonymous)
    {
      const fdb_mstruct_t* child = field->p_strct_type;
      for(uint32_t j = 0; j < child->m_nfields; ++j)
      {
        const fdb_mfield_t* child_field = child->p_fields[j];

        if(strcmp(child_field->m_name,"m_q")==0)
        {
          float* field_ptr = (float*)((char*)raw_pointer + child_field->m_offset);
          ASSERT_EQ(*field_ptr , 4.0f);
          continue;
        }

        if(strcmp(child_field->m_name, "m_t") == 0)
        {
          float* field_ptr = (float*)((char*)raw_pointer + child_field->m_offset);
          ASSERT_EQ(*field_ptr , 5.0f);
          continue;
        }
      }

      continue;
    }

    if(strcmp(field->m_name,"X") == 0)
    {
      const fdb_mstruct_t* child = field->p_strct_type;
      for(uint32_t j = 0; j < child->m_nfields; ++j)
      {
        const fdb_mfield_t* child_field = child->p_fields[j];

        if(strcmp(child_field->m_name,"m_a") == 0)
        {
          float* field_ptr = (float*)((char*)raw_pointer + field->m_offset + child_field->m_offset);
          ASSERT_EQ(*field_ptr , 6.0f);
          continue;
        }

        if(strcmp(child_field->m_name, "m_b") == 0)
        {
          float* field_ptr = (float*)((char*)raw_pointer + field->m_offset + child_field->m_offset);
          ASSERT_EQ(*field_ptr , 6.0f);
          continue;
        }
      }
      continue;
    }
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

