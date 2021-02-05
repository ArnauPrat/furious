
#include "furious.h"

#include <gtest/gtest.h>

TEST(BitTableTest, BitTableWorks) 
{

  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, 
                       KILOBYTES(64), nullptr);
  fdb_tmpbittable_factory_t bt_factory;
  fdb_tmpbittable_factory_init(&bt_factory, &lallocator.m_super);
  fdb_tmpbittable_t bt;
  fdb_tmpbittable_init(&bt, &bt_factory);
  constexpr uint32_t num_elements = 32000;
  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_tmpbittable_add(&bt,i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_tmpbittable_get_bitmap(&bt, i);
    ASSERT_TRUE(fdb_tmpbittable_exists(&bt, i));
  }

  fdb_tmpbittable_clear(&bt);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(fdb_tmpbittable_exists(&bt, i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_tmpbittable_add(&bt, i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_tmpbittable_exists(&bt, i));
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    fdb_tmpbittable_remove(&bt, i);
  }

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_FALSE(fdb_tmpbittable_exists(&bt, i));
  }

  fdb_tmpbittable_clear(&bt);
  fdb_tmpbittable_release(&bt);
  fdb_tmpbittable_factory_release(&bt_factory);
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
  fdb_tmpbittable_factory_t bt_factory;
  fdb_tmpbittable_factory_init(&bt_factory, nullptr);
  fdb_tmpbittable_t bt1;
  fdb_tmpbittable_init(&bt1, &bt_factory);
  fdb_tmpbittable_t bt2;
  fdb_tmpbittable_init(&bt2, &bt_factory);
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
      fdb_tmpbittable_add(&bt2, i);
  }

  fdb_tmpbittable_union(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_tmpbittable_exists(&bt1, i));
  }

  fdb_tmpbittable_union(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    ASSERT_TRUE(fdb_tmpbittable_exists(&bt1, i));
  }

  fdb_tmpbittable_clear(&bt1);
  fdb_tmpbittable_clear(&bt2);

  fdb_tmpbittable_release(&bt1);
  fdb_tmpbittable_release(&bt2);
  fdb_tmpbittable_factory_release(&bt_factory);
}

TEST(BitTableTest, BitTableDifference) 
{
  fdb_tmpbittable_factory_t bt_factory;
  fdb_tmpbittable_factory_init(&bt_factory, nullptr);
  fdb_tmpbittable_t bt1;
  fdb_tmpbittable_init(&bt1, &bt_factory);
  fdb_tmpbittable_t bt2;
  fdb_tmpbittable_init(&bt2, &bt_factory);
  constexpr uint32_t num_elements = 1000000;

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      fdb_tmpbittable_add(&bt2, i);
    }
    fdb_tmpbittable_add(&bt1, i);
  }

  fdb_tmpbittable_difference(&bt1, &bt2);

  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    if(i%2 == 0)
    {
      ASSERT_FALSE(fdb_tmpbittable_exists(&bt1, i));
      ASSERT_TRUE(fdb_tmpbittable_exists(&bt2, i));
    }
    else
    {
      ASSERT_TRUE(fdb_tmpbittable_exists(&bt1,i));
    }
  }

  fdb_tmpbittable_clear(&bt1);
  fdb_tmpbittable_clear(&bt2);
  fdb_tmpbittable_release(&bt1);
  fdb_tmpbittable_release(&bt2);
  fdb_tmpbittable_factory_release(&bt_factory);
}

TEST(BitTableTest, BitTableSteps) 
{
  fdb_tmpbittable_factory_t bt_factory;
  fdb_tmpbittable_factory_init(&bt_factory, nullptr);
  fdb_tmpbittable_t bt;
  fdb_tmpbittable_init(&bt,&bt_factory);
  constexpr uint32_t MAX_ELEMENTS = 1600;
  uint32_t stride = 21;
  uint32_t offset = 21604;
  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    fdb_tmpbittable_add(&bt,i*stride + offset);
  }

  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    const fdb_bitmap_t* bt_block = fdb_tmpbittable_get_bitmap(&bt, i*stride+offset);
    ASSERT_NE(bt_block, nullptr);
  }

  //ASSERT_EQ(btree_num_allocations, 0);
  fdb_tmpbittable_release(&bt);
  fdb_tmpbittable_factory_release(&bt_factory);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

