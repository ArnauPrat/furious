
#include "furious.h"

#include <gtest/gtest.h>

namespace furious {

TEST(BitTableTest, BitTableWorks) 
{

  BitTable bittable;
  constexpr uint32_t num_elements = 1000000;
  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    bittable.add(i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(bittable.exists(i));
  }

  bittable.clear();

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(bittable.exists(i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    bittable.add(i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(bittable.exists(i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    bittable.remove(i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(bittable.exists(i));
  }

  bittable.clear();
}

TEST(BitTableTest, BitTableClear) 
{
  BitTable bittable;
  constexpr uint32_t num_elements = 100000;
  for (uint32_t i = 0; 
       i < num_elements;
       ++i) 
  {
    if(i % 10000 == 0)
    {
      bittable.clear();
    }
    bittable.add(i);
  }
  bittable.clear();
  
}


TEST(BitTableTest, BitTableUnion) 
{
  BitTable* bittable1 = new BitTable();
  BitTable* bittable2 = new BitTable();
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
      bittable2->add(i);
  }

  bittable_union(bittable1, bittable2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(bittable1->exists(i));
  }

  bittable1->clear();
  bittable2->clear();

  delete bittable1;
  delete bittable2;
}

TEST(BitTableTest, BitTableDifference) 
{
  BitTable* bittable1 = new BitTable();
  BitTable* bittable2 = new BitTable();
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      bittable2->add(i);
    }
    bittable1->add(i);
  }

  bittable_difference(bittable1, bittable2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      ASSERT_FALSE(bittable1->exists(i));
      ASSERT_TRUE(bittable2->exists(i));
    }
    else
    {
      ASSERT_TRUE(bittable1->exists(i));
    }
  }

  bittable1->clear();
  bittable2->clear();
  delete bittable1;
  delete bittable2;
}
}


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

