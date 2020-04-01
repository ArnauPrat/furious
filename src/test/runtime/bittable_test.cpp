
#include "furious.h"

#include <gtest/gtest.h>

TEST(BitTableTest, BitTableWorks) 
{

  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, 
                       KILOBYTES(64), nullptr);
  fdb_bittable_t bt;
  fdb_bittable_init(&bt, &lallocator.m_super);
  constexpr uint32_t num_elements = 32000;
  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_bittable_add(&bt,i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_bittable_get_bitmap(&bt, i);
    ASSERT_TRUE(fdb_bittable_exists(&bt, i));
  }

  fdb_bittable_clear(&bt);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(fdb_bittable_exists(&bt, i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_bittable_add(&bt, i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_bittable_exists(&bt, i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_bittable_remove(&bt, i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(fdb_bittable_exists(&bt, i));
  }

  fdb_bittable_clear(&bt);
  fdb_bittable_release(&bt);
  fdb_stack_alloc_release(&lallocator);
}

/*TEST(BitTableTest, BitTableClear) 
{
  fdb_bittable_t bt = fdb_bittable_init();
  constexpr uint32_t num_elements = 100000;
  for (uint32_t i = 0; 
       i < num_elements;
       ++i) 
  {
    if(i % 10000 == 0)
    {
      fdb_bittable_clear(&bt);
    }
    fdb_bittable_add(&bt, i);
  }
  fdb_bittable_clear(&bt);
}
*/


TEST(BitTableTest, BitTableUnion) 
{
  fdb_bittable_t bt1;
  fdb_bittable_init(&bt1, nullptr);
  fdb_bittable_t bt2;
  fdb_bittable_init(&bt2, nullptr);
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
      fdb_bittable_add(&bt2, i);
  }

  fdb_bittable_union(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_bittable_exists(&bt1, i));
  }

  fdb_bittable_union(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_bittable_exists(&bt1, i));
  }

  fdb_bittable_clear(&bt1);
  fdb_bittable_clear(&bt2);

  fdb_bittable_release(&bt1);
  fdb_bittable_release(&bt2);
}

TEST(BitTableTest, BitTableDifference) 
{
  fdb_bittable_t bt1;
  fdb_bittable_init(&bt1, nullptr);
  fdb_bittable_t bt2;
  fdb_bittable_init(&bt2, nullptr);
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      fdb_bittable_add(&bt2, i);
    }
    fdb_bittable_add(&bt1, i);
  }

  fdb_bittable_difference(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      ASSERT_FALSE(fdb_bittable_exists(&bt1, i));
      ASSERT_TRUE(fdb_bittable_exists(&bt2, i));
    }
    else
    {
      ASSERT_TRUE(fdb_bittable_exists(&bt1,i));
    }
  }

  fdb_bittable_clear(&bt1);
  fdb_bittable_clear(&bt2);
  fdb_bittable_release(&bt1);
  fdb_bittable_release(&bt2);
}

TEST(BitTableTest, BitTableSteps) 
{
  fdb_bittable_t bt;
  fdb_bittable_init(&bt,nullptr);
  constexpr uint32_t MAX_ELEMENTS = 1600;
  uint32_t stride = 21;
  uint32_t offset = 21604;
  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    fdb_bittable_add(&bt,i*stride + offset);
  }

  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    const fdb_bitmap_t* bt_block = fdb_bittable_get_bitmap(&bt, i*stride+offset);
    ASSERT_NE(bt_block, nullptr);
  }

  //ASSERT_EQ(btree_num_allocations, 0);
  fdb_bittable_release(&bt);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

