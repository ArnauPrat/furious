
#include "../../common/bitmap.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

TEST(BitmapTest, BitmapTest ) 
{
  uint32_t BITMAP_SIZE = 304;
  Bitmap bitmap(BITMAP_SIZE);
  ASSERT_EQ(bitmap.num_set(), 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(bitmap.is_set(i), false);
    bitmap.set(i);
  }

  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(bitmap.is_set(i), true);
    bitmap.unset(i);
    ASSERT_EQ(bitmap.is_set(i), false);
  }

  ASSERT_EQ(bitmap.num_set(), 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; i+=2)
  {
    ASSERT_EQ(bitmap.is_set(i), false);
    bitmap.set(i);
    ASSERT_EQ(bitmap.is_set(i), true);
  }

  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap.is_set(i), true);
      bitmap.unset(i);
      ASSERT_EQ(bitmap.is_set(i), false);
    }
    else
    {
      ASSERT_EQ(bitmap.is_set(i), false);
      bitmap.set(i);
      ASSERT_EQ(bitmap.is_set(i), true);
    }
  }

  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE/2);

  bitmap.set_negate();

  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap.is_set(i), true);
      bitmap.unset(i);
      ASSERT_EQ(bitmap.is_set(i), false);
    }
    else
    {
      ASSERT_EQ(bitmap.is_set(i), false);
      bitmap.set(i);
      ASSERT_EQ(bitmap.is_set(i), true);
    }
  }
  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE/2);

  Bitmap bitmap2(BITMAP_SIZE);
  bitmap2.set_bitmap(&bitmap);

  bitmap2.set_negate();
  bitmap.set_or(&bitmap2);
  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE);
  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
      ASSERT_EQ(bitmap.is_set(i), true);
  }

  bitmap.set_and(&bitmap2);
  ASSERT_EQ(bitmap.num_set(), BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap.is_set(i), true);
    }
    else
    {
      ASSERT_EQ(bitmap.is_set(i), false);
    }
  }

}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

