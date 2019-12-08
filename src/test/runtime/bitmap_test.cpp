
#include "../../common/bitmap.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

TEST(BitmapTest, BitmapTest ) 
{
  constexpr uint32_t BITMAP_SIZE = 304;
  bitmap_t<BITMAP_SIZE> bitmap = {0};
  ASSERT_EQ(bitmap.m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
    bitmap_set(&bitmap, i);
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
    bitmap_unset(&bitmap, i);
    ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
  }

  ASSERT_EQ(bitmap.m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; i+=2)
  {
    ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
    bitmap_set(&bitmap, i);
    ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
      bitmap_unset(&bitmap, i);
      ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
    }
    else
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
      bitmap_set(&bitmap, i);
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
    }
  }

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  bitmap_negate(&bitmap);

  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
      bitmap_unset(&bitmap, i);
      ASSERT_EQ(bitmap_is_set(&bitmap,i), false);
    }
    else
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
      bitmap_set(&bitmap, i);
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
    }
  }
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  bitmap_t<BITMAP_SIZE> bitmap2 = {0};
  bitmap_set_bitmap(&bitmap2, &bitmap);

  bitmap_negate(&bitmap2);
  bitmap_set_or(&bitmap, &bitmap2);
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE);
  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
  }

  bitmap_set_and(&bitmap, &bitmap2);
  ASSERT_EQ(bitmap.m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), true);
    }
    else
    {
      ASSERT_EQ(bitmap_is_set(&bitmap, i), false);
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

