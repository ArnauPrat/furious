
#include "furious.h"

#include <gtest/gtest.h>


TEST(fdb_stack_alloc_test, fdb_stack_alloc_test) 
{
  fdb_stack_alloc_t alloc;
  fdb_stack_alloc_init(&alloc, KILOBYTES(4), nullptr);

  constexpr uint32_t num_alignments = 10;
  uint32_t alignments[num_alignments] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  constexpr uint32_t num_allocations = 250;
  uint32_t allocation_sizes[num_allocations];
  void*    allocation_addrs[num_allocations];
  uint32_t    allocation_alignments[num_allocations];
  srand(time(NULL));
  for (uint32_t i = 0; i < num_allocations; ++i) 
  {
    allocation_alignments[i] = (uint32_t)alignments[(uint32_t)rand() % num_alignments];
    allocation_sizes[i] = (uint32_t)(rand() % (KILOBYTES(4) - sizeof(void*) - (allocation_alignments[i] < FDB_MIN_ALIGNMENT ? FDB_MIN_ALIGNMENT : allocation_alignments[i]))) + 1;
    allocation_addrs[i] = nullptr;
  }

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    allocation_addrs[i] = fdb_stack_alloc_alloc(&alloc, 
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
    fdb_stack_alloc_free(&alloc, allocation_addrs[i]);
  }

  fdb_stack_alloc_release(&alloc);
}

TEST(fdb_stack_alloc_test, fdb_stack_alloc_test_pop) 
{
  fdb_stack_alloc_t alloc;
  fdb_stack_alloc_init(&alloc, KILOBYTES(4), nullptr);

  uint32_t alignment = 32;
  uint32_t alloc_size = 48;
  uint32_t num_allocs = 1024;
  void** allocations = new void*[num_allocs];
  for(uint32_t i = 0; i < num_allocs; ++i)
  {
    allocations[i] = fdb_stack_alloc_alloc(&alloc, 
                                        alignment, 
                                        alloc_size, 
                                        FDB_NO_HINT);
  }

  for(uint32_t i = 0; i < num_allocs; ++i)
  {
    fdb_stack_alloc_pop(&alloc, 
                        allocations[num_allocs-1-i]);
  }
  delete [] allocations;
  fdb_stack_alloc_release(&alloc);
}


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

