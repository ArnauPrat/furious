
#include "furious.h"
#include "basic_test_header.h"

#include <gtest/gtest.h>
#include <set>
#include <vector>
#include <iostream>


TEST(ExpandTest, ExpandTest ) 
{
  furious::Database* database = new furious::Database();
  database->start_webserver("localhost", 
                            "8080");
  furious::__furious_init(database);
  furious::__furious_frame(0.1,database);
  furious::__furious_release();

  delete database;
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

