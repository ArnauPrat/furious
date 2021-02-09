
#include "furious.h"

#include <gtest/gtest.h>


TEST(fdb_bcluster_Test, fdb_bcluster_Test) 
{
  struct fdb_tmptable_factory_t tmptable_factory;
  fdb_tmptable_factory_init(&tmptable_factory, 
                            nullptr);
  struct fdb_tmptable_t tmptable;
  fdb_tmptable_init(&tmptable, 
                    &tmptable_factory, 
                    "table", 
                    0, 
                    sizeof(uint32_t),
                    NULL);
  struct fdb_tmptable_block_t tblock;
  fdb_tmptable_block_init(&tblock, 
                          &tmptable, 
                          64, 
                          sizeof(uint32_t));

  struct fdb_bcluster_factory_t cluster_factory;
  fdb_bcluster_factory_init(&cluster_factory, 
                            nullptr);

  fdb_bcluster_t cluster;
  fdb_bcluster_init(&cluster, &cluster_factory);
  fdb_bcluster_append_tmptable_block(&cluster, &tblock);
  ASSERT_EQ(cluster.p_blocks[0], tblock.p_data);
  fdb_bcluster_append_global(&cluster, (void*)(0xff), 4);
  ASSERT_EQ(cluster.p_blocks[1], (void*)0xff);
  ASSERT_EQ(cluster.m_sizes[1], 4);

  struct fdb_tmptable_block_t tblock2;
  fdb_tmptable_block_init(&tblock2, 
                          &tmptable, 
                          64, 
                          sizeof(uint32_t));
  fdb_bcluster_t cluster2;
  fdb_bcluster_init(&cluster2, &cluster_factory);
  fdb_bcluster_append_tmptable_block(&cluster2, &tblock2);

  fdb_bcluster_append_cluster(&cluster2, &cluster);
  ASSERT_EQ(cluster2.p_blocks[0], tblock2.p_data);
  ASSERT_EQ(cluster2.p_blocks[1], tblock.p_data);
  ASSERT_EQ(cluster2.p_blocks[2], (void*)0xff);
  fdb_bcluster_release(&cluster);
  fdb_bcluster_release(&cluster);

  fdb_bcluster_factory_release(&cluster_factory);
  fdb_tmptable_release(&tmptable);
  fdb_tmptable_factory_release(&tmptable_factory);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

