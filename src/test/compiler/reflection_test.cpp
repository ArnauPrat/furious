
#include "furious.h"
#include "reflection_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(ReflectionTest, ReflectionTest ) 
{
  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");
  furious::__furious_init(database);

  furious::Entity entity = furious::create_entity(database);
  FURIOUS_ADD_COMPONENT(entity, TestComponent);
  TestComponent* test_component = FURIOUS_GET_COMPONENT(entity, TestComponent);
  test_component->m_x = 1.0f;
  test_component->m_y = 2.0f; 
  test_component->m_z = 3.0f;
  test_component->m_q = 4.0f;
  test_component->m_t = 5.0f;
  test_component->X.m_a = 6.0f;
  test_component->X.m_b = 6.0f;

  furious::__furious_frame(0.1,database, nullptr);

  void* raw_pointer = test_component;
  const furious::ReflData* refl_data = database->get_refl_data<TestComponent>();
  ASSERT_TRUE(refl_data != nullptr);
  for(uint32_t i = 0; i < refl_data->m_fields.size(); ++i)
  {
    const furious::ReflField* field = &refl_data->m_fields[i];
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
      const furious::ReflData* child = field->p_strct_type.get();
      for(uint32_t j = 0; j < child->m_fields.size(); ++j)
      {
        const furious::ReflField* child_field = &child->m_fields[j];

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
      const furious::ReflData* child = field->p_strct_type.get();
      for(uint32_t j = 0; j < child->m_fields.size(); ++j)
      {
        const furious::ReflField* child_field = &child->m_fields[j];

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

  furious::__furious_release();

  delete database;
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

