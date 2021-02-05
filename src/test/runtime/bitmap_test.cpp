
#include "../../common/bitmap.h"
#include "../../common/memory/stack_allocator.h"

#include <gtest/gtest.h>
#include <set>

TEST(BitmapTest, BitmapTest ) 
{
  constexpr uint32_t BITMAP_SIZE = 304;
  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, KILOBYTES(4), NULL);
  fdb_bitmap_factory_t bitmap_factory;
  fdb_bitmap_factory_init(&bitmap_factory, BITMAP_SIZE, &lallocator.m_super);
  fdb_bitmap_t bitmap;
  fdb_bitmap_init(&bitmap, &bitmap_factory);
  ASSERT_EQ(bitmap.m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
    fdb_bitmap_set(&bitmap, i);
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
    fdb_bitmap_unset(&bitmap, i);
    ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
  }

  ASSERT_EQ(bitmap.m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; i+=2)
  {
    ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
    fdb_bitmap_set(&bitmap, i);
    ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
      fdb_bitmap_unset(&bitmap, i);
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
    }
    else
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
      fdb_bitmap_set(&bitmap, i);
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
    }
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  fdb_bitmap_negate(&bitmap);

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
      fdb_bitmap_unset(&bitmap, i);
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap,i), false);
    }
    else
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
      fdb_bitmap_set(&bitmap, i);
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
    }
  }
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  fdb_bitmap_t bitmap2;
  fdb_bitmap_init(&bitmap2, 
                  &bitmap_factory);
  fdb_bitmap_set_bitmap(&bitmap2, &bitmap);

  fdb_bitmap_negate(&bitmap2);
  fdb_bitmap_set_or(&bitmap, &bitmap2);
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE);
  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
  }

  fdb_bitmap_set_and(&bitmap, &bitmap2);
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), true);
    }
    else
    {
      ASSERT_EQ(fdb_bitmap_is_set(&bitmap, i), false);
    }
  }
  fdb_bitmap_release(&bitmap);
  fdb_bitmap_release(&bitmap2);
  fdb_bitmap_factory_release(&bitmap_factory);
  fdb_stack_alloc_release(&lallocator);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

