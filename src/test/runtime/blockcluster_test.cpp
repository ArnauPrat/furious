
#include "furious.h"

#include <gtest/gtest.h>


TEST(fdb_bcluster_Test, fdb_bcluster_Test) 
{
  fdb_table_block_t tblock;
  fdb_table_block_init(&tblock, 
                       64, 
                       sizeof(long), 
                       fdb_get_global_mem_allocator(),
                       fdb_get_global_mem_allocator());

  fdb_bcluster_t cluster;
  fdb_bcluster_init(&cluster, nullptr);
  fdb_bcluster_append_block(&cluster, &tblock);
  ASSERT_EQ(fdb_bcluster_get_tblock(&cluster, 0), &tblock);
  fdb_bcluster_append_global(&cluster, (void*)(0xff));
  ASSERT_EQ(fdb_bcluster_get_global(&cluster, 1), (void*)0xff);

  fdb_table_block_t tblock2;
  fdb_table_block_init(&tblock2, 
                       64, 
                       sizeof(int), 
                       fdb_get_global_mem_allocator(),
                       fdb_get_global_mem_allocator());

  fdb_bcluster_t cluster2;
  fdb_bcluster_init(&cluster2, nullptr);
  fdb_bcluster_append_block(&cluster2, &tblock2);

  fdb_bcluster_append_cluster(&cluster2, &cluster);
  ASSERT_EQ(fdb_bcluster_get_tblock(&cluster2, 0), &tblock2);
  ASSERT_EQ(fdb_bcluster_get_tblock(&cluster2,1), &tblock);
  ASSERT_EQ(fdb_bcluster_get_global(&cluster2,2), (void*)0xff);
  fdb_bcluster_release(&cluster, fdb_get_global_mem_allocator());
  fdb_bcluster_release(&cluster2, fdb_get_global_mem_allocator());
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

