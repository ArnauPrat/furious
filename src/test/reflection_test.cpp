#include "../furious.h"
#include <gtest/gtest.h>

namespace furious {

struct ComponentA {
  int32_t m_field;

  static std::string name() {
    return "component_a";
  }
};

struct ComponentB {
  int32_t m_field;

  static std::string name() {
    return "component_b";
  }
};


class TestSystem {
public:
  TestSystem(uint32_t val) : m_val{val} {}
  virtual ~TestSystem() = default;

  void run(Context* context, 
           int32_t id, 
           ComponentA* componentA, 
           const ComponentB* componentB ) {
    componentA->m_field = componentB->m_field*m_val;
  }

  uint32_t m_val;
};

extern const char parent_str[] = "parent";

TEST(ReflectionTest,ReflectionWorks) {

  furious::init();

  Ref<ComponentA, parent_str> test(nullptr);

  ASSERT_EQ(type_name<ComponentA>::name(), "component_a");
  ASSERT_EQ(type_name<ComponentB>::name(), "component_b");

  bool result = type_name<Ref<ComponentA, parent_str>>::name() == "component_a";
  ASSERT_TRUE(result);
  result = type_name<Ref<ComponentB, parent_str> >::name() == "component_b";
  ASSERT_TRUE(result);

  ASSERT_EQ(access_type<const ComponentA*>::type(), ComAccessType::E_READ);
  ASSERT_EQ(access_type<ComponentB>::type(), ComAccessType::E_WRITE);

  result = access_type<Ref<const ComponentA*, parent_str>>::type() == ComAccessType::E_READ;
  result = access_type<Ref<ComponentB, parent_str>>::type() == ComAccessType::E_WRITE;

  furious::release();
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

