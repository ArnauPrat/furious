
#include "../../common/hashtable.h"

#include <gtest/gtest.h>
#include <set>


TEST(HashtableTest, HashtableTest ) 
{
  fdb_hashtable_t hashtable;
  fdb_hashtable_init(&hashtable, 
                     512, 
                     nullptr);
  constexpr uint32_t num_elements = 100000;
  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    fdb_hashtable_add(&hashtable, 
                  i,
                  (void*)(uint64_t)i);
  }

  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    void* ptr = fdb_hashtable_get(&hashtable, 
                  i);
    ASSERT_EQ(ptr, (void*)(uint64_t)i);
  }


  fdb_hashtable_release(&hashtable);
}

TEST(HashtableTest, HashtableIteratorTest ) 
{
  fdb_hashtable_t hashtable;
  fdb_hashtable_init(&hashtable, 512, nullptr);
  constexpr uint32_t num_elements = 100000;
  for(uint32_t i = 0; 
      i < num_elements;
      ++i)
  {
    fdb_hashtable_add(&hashtable, 
                  i,
                  (void*)(uint64_t)i);
  }

  bool found[num_elements];
  memset(found, 0, sizeof(bool)*num_elements);

  fdb_hashtable_iter_t iter = fdb_hashtable_iter_init(&hashtable);
  while(fdb_hashtable_iter_has_next(&iter))
  {
    fdb_hashtable_entry_t entry = fdb_hashtable_iter_next(&iter);
    found[entry.m_key] = true;
  }
  fdb_hashtable_iter_release(&iter);

  bool all_true = true;
  for(uint32_t i = 0;
      i < num_elements; 
      ++i)
  {
    all_true &= found[i];
  }

  ASSERT_TRUE(all_true);

  fdb_hashtable_release(&hashtable);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

