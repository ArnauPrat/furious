
#include "furious.h"

#include <gtest/gtest.h>

namespace furious 
{

TEST(stack_alloc_test, stack_alloc_test) 
{
  mem_allocator_t alloc = stack_alloc_create();

  constexpr uint32_t num_alignments = 10;
  uint32_t alignments[num_alignments] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  constexpr uint32_t num_allocations = 250;
  uint32_t allocation_sizes[num_allocations];
  void*    allocation_addrs[num_allocations];
  uint32_t    allocation_alignments[num_allocations];
  srand(time(NULL));
  for (uint32_t i = 0; i < num_allocations; ++i) 
  {
    allocation_sizes[i] = (uint32_t)(rand() % 65536);
    allocation_addrs[i] = nullptr;
    allocation_alignments[i] = (uint32_t)alignments[(uint32_t)rand() % num_alignments];
  }

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    allocation_addrs[i] = mem_alloc(&alloc, 
                                    allocation_alignments[i], 
                                    allocation_sizes[i],
                                    -1);
    memset(allocation_addrs[i], i % 256, allocation_sizes[i]);
  }

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    char signature = i % 256;
    char* data = (char*)allocation_addrs[i];
    for(uint32_t j = 0; j < allocation_sizes[i]; ++j)
    {
      ASSERT_EQ(data[j], signature);
    }
    uint32_t modulo = ((uint64_t)allocation_addrs[i]) % allocation_alignments[i];
    ASSERT_EQ(modulo, 0);
    mem_free(&alloc, allocation_addrs[i]);
  }

  stack_alloc_destroy(&alloc);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

