
#include "../../common/hashtable.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

TEST(HashtableTest, HashtableTest ) 
{
  hashtable_t hashtable = hashtable_create(512);
  constexpr uint32_t num_elements = 100000;
  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    hashtable_add(&hashtable, 
                  i,
                  (void*)(uint64_t)i);
  }

  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    void* ptr = hashtable_get(&hashtable, 
                  i);
    ASSERT_EQ(ptr, (void*)(uint64_t)i);
  }


  hashtable_destroy(&hashtable);
}

TEST(HashtableTest, HashtableIteratorTest ) 
{
  hashtable_t hashtable = hashtable_create(512);
  constexpr uint32_t num_elements = 100000;
  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    hashtable_add(&hashtable, 
                  i,
                  (void*)(uint64_t)i);
  }

  bool found[num_elements];
  memset(found, 0, sizeof(bool)*num_elements);

  hashtable_iter_t iter = hashtable_iter_create(&hashtable);
  while(hashtable_iter_has_next(&iter))
  {
    hashtable_entry_t entry = hashtable_iter_next(&iter);
    found[entry.m_key] = true;
  }
  hashtable_iter_destroy(&iter);

  bool all_true = true;
  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    all_true &= found[i];
  }

  ASSERT_TRUE(all_true);

  hashtable_destroy(&hashtable);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

