
#include "furious.h"

#include <gtest/gtest.h>

namespace furious 
{

TEST(pool_alloc_test, only_allocs) 
{

  constexpr uint32_t num_alignments = 10;
  uint32_t alignments[num_alignments] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  constexpr uint32_t num_chunks = 5;
  uint32_t chunk_sizes[num_chunks] = {KILOBYTES(4),
                                      KILOBYTES(8),
                                      KILOBYTES(16),
                                      KILOBYTES(32),
                                      KILOBYTES(64)};
  constexpr uint32_t num_blocks = 4;
  uint32_t block_sizes[num_blocks] = {BYTES(32), 
                                      BYTES(64),
                                      BYTES(128),
                                      BYTES(256)};
    

  for (uint32_t i = 0; i < num_alignments; ++i) 
  {
    for (uint32_t j = 0; j < num_chunks; ++j) 
    {
      for (uint32_t k = 0; k < num_blocks; ++k) 
      {
        mem_allocator_t alloc = pool_alloc_create(alignments[i],
                                                  block_sizes[k],
                                                  chunk_sizes[j]);
        // NOTE(Arnau) This number cannot exceed 256
        constexpr uint32_t num_allocations = 256;
        DynArray<char*> allocations;
        for(uint32_t q = 0; q < num_allocations; ++q)
        {
          void* ptr = mem_alloc(&alloc, 
                                alignments[i], 
                                block_sizes[k],
                                FURIOUS_NO_HINT);
          memset(ptr, q, block_sizes[k]);
          ASSERT_EQ((uint64_t)ptr % alignments[i], 0);
          allocations.append((char*)ptr);
        }

        for(uint32_t q = 1; q < num_allocations; ++q)
        {
          char* ptr1 = allocations[q-1];
          char* ptr2 = allocations[q];
          ASSERT_TRUE(((uint64_t)ptr2 - (uint64_t)ptr1) >= block_sizes[k]);
        }

        for(uint32_t q = 0; q < num_allocations; ++q)
        {
          char* ptr =allocations[q];
          //printf("BLOCK: %u size:%u ptr:%lu\n", q, block_sizes[k], (uint64_t)ptr);
          for(uint32_t t = 0; t < block_sizes[k]; ++t)
          {
            //printf("%u %u %u\n", t, (uint8_t)ptr[t], (uint8_t)q);
            ASSERT_EQ((uint8_t)ptr[t], (uint8_t)q);
          }
        }

        pool_alloc_destroy(&alloc);
      }
    }
  }

}

TEST(pool_alloc_test, allocs_and_frees_test) 
{

  constexpr uint32_t num_alignments = 10;
  uint32_t alignments[num_alignments] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  constexpr uint32_t num_chunks = 5;
  uint32_t chunk_sizes[num_chunks] = {KILOBYTES(4),
                                      KILOBYTES(8),
                                      KILOBYTES(16),
                                      KILOBYTES(32),
                                      KILOBYTES(64)};
  constexpr uint32_t num_blocks = 4;
  uint32_t block_sizes[num_blocks] = {BYTES(32), 
                                      BYTES(64),
                                      BYTES(128),
                                      BYTES(256)};
    

  for (uint32_t i = 0; i < num_alignments; ++i) 
  {
    for (uint32_t j = 0; j < num_chunks; ++j) 
    {
      for (uint32_t k = 0; k < num_blocks; ++k) 
      {
        mem_allocator_t alloc = pool_alloc_create(alignments[i],
                                                   block_sizes[k],
                                                   chunk_sizes[j]);
        constexpr uint32_t num_allocations = 1024;
        for(uint32_t q = 0; q < num_allocations; ++q)
        {
          void* ptr = mem_alloc(&alloc, 
                                alignments[i], 
                                block_sizes[k],
                                FURIOUS_NO_HINT);
          memset(ptr, 0, block_sizes[k]);
          ASSERT_EQ((uint64_t)ptr % alignments[i], 0);
          if(q % 3 == 0)
          {
            mem_free(&alloc, ptr);
          }
        }

        pool_alloc_destroy(&alloc);
      }
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

