
#include "furious.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

TEST(MemoryTest, NUMA ) {
  void* ptr = mem_alloc(&global_mem_allocator, 
                        64, 
                        sizeof(int32_t)*1024, 
                        0);
  ASSERT_NE(ptr, nullptr);
  mem_free(&global_mem_allocator, 
           ptr);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

