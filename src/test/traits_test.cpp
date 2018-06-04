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

TEST(TraitsTest,TraitsWorks) {

  furious::init();

  Ref<ComponentA, parent_str> test(nullptr);

  // testing name trait
  ASSERT_EQ(type_name<ComponentA>::name(), "component_a");
  ASSERT_EQ(type_name<ComponentB>::name(), "component_b");

  bool result = type_name<Ref<ComponentA, parent_str>>::name() == "component_a";
  ASSERT_TRUE(result);
  result = type_name<Ref<ComponentB, parent_str> >::name() == "component_b";
  ASSERT_TRUE(result);

  // testing accesstype trait
  ASSERT_EQ(access_type<const ComponentA*>::type(), ComAccessType::E_READ);
  ASSERT_EQ(access_type<ComponentB>::type(), ComAccessType::E_WRITE);

  result = access_type<Ref<const ComponentA*, parent_str>>::type() == ComAccessType::E_READ;
  ASSERT_TRUE(result);
  result = access_type<Ref<ComponentB, parent_str>>::type() == ComAccessType::E_WRITE;
  ASSERT_TRUE(result);

  // testing is_ref trait
  ASSERT_EQ(is_ref<ComponentA>::value(), false);
  ASSERT_EQ(is_ref<ComponentB>::value(), false);

  result = is_ref<Ref<ComponentA, parent_str>>::value() == true;
  ASSERT_TRUE(result);
  result = is_ref<Ref<ComponentB, parent_str>>::value() == true;
  ASSERT_TRUE(result);

  // testing ref_name trait
  ASSERT_EQ(ref_name<ComponentA>::value(), nullptr);
  ASSERT_EQ(ref_name<ComponentB>::value(), nullptr);

  result = ref_name<Ref<ComponentA, parent_str>>::value() == parent_str;
  ASSERT_TRUE(result);
  result = ref_name<Ref<ComponentB, parent_str>>::value() == parent_str;
  ASSERT_TRUE(result);

  furious::release();
}
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

