
#include "../../common/dyn_array.h"

#include <gtest/gtest.h>

namespace furious {

struct test_t
{
  std::string m_str = "";
};

DynArray<test_t> 
create_array()
{
  DynArray<test_t> array;
  for(uint32_t i = 0; i < 100; ++i)
  {
    test_t t;
    t.m_str = "TEST";
    array.append(t);
  }
  return array;
}

TEST(DynArray, DynArrayTest ) 
{
  DynArray<test_t> array = create_array();
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

