
#include "../memory/memory.h"

#include <gtest/gtest.h>
#include <set>

namespace furious {

TEST(MemoryTest, NUMA ) {
  void* ptr = numa_alloc(0, sizeof(int32_t)*1024);
  ASSERT_NE(ptr, nullptr);
  numa_free(ptr);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

