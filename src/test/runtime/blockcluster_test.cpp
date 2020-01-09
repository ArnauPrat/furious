
#include "furious.h"

#include <gtest/gtest.h>

namespace furious {

TEST(block_cluster_Test, block_cluster_Test) 
{
  table_block_t tblock = table_block_create(64, 
                                            sizeof(long), 
                                            &global_mem_allocator,
                                            &global_mem_allocator);

  block_cluster_t cluster = block_cluster_create(nullptr);
  block_cluster_append(&cluster, &tblock);
  ASSERT_EQ(block_cluster_get_tblock(&cluster, 0), &tblock);
  block_cluster_append_global(&cluster, (void*)(0xff));
  ASSERT_EQ(block_cluster_get_global(&cluster, 1), (void*)0xff);

  table_block_t tblock2 = table_block_create(64, 
                                             sizeof(int), 
                                             &global_mem_allocator,
                                             &global_mem_allocator);
  block_cluster_t cluster2 = block_cluster_create(nullptr);
  block_cluster_append(&cluster2, &tblock2);

  block_cluster_append(&cluster2, &cluster);
  ASSERT_EQ(block_cluster_get_tblock(&cluster2, 0), &tblock2);
  ASSERT_EQ(block_cluster_get_tblock(&cluster2,1), &tblock);
  ASSERT_EQ(block_cluster_get_global(&cluster2,2), (void*)0xff);
  block_cluster_destroy(&cluster, &global_mem_allocator);
  block_cluster_destroy(&cluster2, &global_mem_allocator);
}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

