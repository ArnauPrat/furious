
#include "furious.h"
#include "common/platform.h"
#include "common/memory/page_allocator.h"

#include <gtest/gtest.h>

TEST(fdb_pool_alloc_test, test) 
{
  fdb_page_alloc_t palloc;
  fdb_page_alloc_init(&palloc, GIGABYTES(1), KILOBYTES(4), KILOBYTES(512));

  uint32_t numAllocs = 1000;
  void* sallocs[numAllocs];
  void* lallocs[numAllocs];
  for(uint32_t i = 0; i < numAllocs; ++i)
  {
    sallocs[i] = fdb_page_alloc_alloc(&palloc, 
                                      64, 
                                      KILOBYTES(4), 
                                      FDB_NO_HINT);

    ASSERT_EQ((uint64_t)sallocs[i] % KILOBYTES(4), 0);

    lallocs[i] = fdb_page_alloc_alloc(&palloc, 
                                      64, 
                                      KILOBYTES(512), 
                                      FDB_NO_HINT);
    
    ASSERT_EQ((uint64_t)lallocs[i] % KILOBYTES(512), 0);
  }

  for(uint32_t i = 0; i < numAllocs/2; ++i)
  {
    fdb_page_alloc_free(&palloc,sallocs[i]);
    fdb_page_alloc_free(&palloc,lallocs[i]);
  }

  for(uint32_t i = 0; i < numAllocs/2; ++i)
  {
    sallocs[i] = fdb_page_alloc_alloc(&palloc, 
                                      64, 
                                      KILOBYTES(4), 
                                      FDB_NO_HINT);

    ASSERT_EQ((uint64_t)sallocs[i] % KILOBYTES(4), 0);

    lallocs[i] = fdb_page_alloc_alloc(&palloc, 
                                      64, 
                                      KILOBYTES(512), 
                                      FDB_NO_HINT);
    
    ASSERT_EQ((uint64_t)lallocs[i] % KILOBYTES(512), 0);
  }

  for(uint32_t i = 0; i < numAllocs; ++i)
  {
    fdb_page_alloc_free(&palloc,sallocs[i]);
    fdb_page_alloc_free(&palloc,lallocs[i]);
  }

  fdb_page_alloc_release(&palloc);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

