
#include "furious.h"

#include <gtest/gtest.h>

namespace furious {

TEST(BlockClusterTest, BlockClusterTest) 
{

  TBlock tblock(64, sizeof(long));

  BlockCluster cluster(&tblock);
  ASSERT_EQ(cluster.get_tblock(0), &tblock);
  cluster.append_global((void*)(0xff));
  ASSERT_EQ(cluster.get_global(1), (void*)0xff);

  TBlock tblock2(64, sizeof(int));
  BlockCluster cluster2(&tblock2);

  cluster2.append(&cluster);
  ASSERT_EQ(cluster2.get_tblock(0), &tblock2);
  ASSERT_EQ(cluster2.get_tblock(1), &tblock);
  ASSERT_EQ(cluster2.get_global(2), (void*)0xff);

}

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

