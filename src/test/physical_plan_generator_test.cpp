

#include "../data/physical_plan_generator.h"
#include "../data/execution_engine.h"
#include "../data/static_system.h"
#include "data_test.h"

#include <gtest/gtest.h>

namespace furious {

    class PhysicalPlanGeneratorTest : public DataTest {

    };

    class TestSystem : public StaticSystem<ComponentA> {
      public:
        TestSystem(SystemId id) : StaticSystem(id) {}
        virtual ~TestSystem() = default;

        void run(ComponentA& component) override {
          component.field1_ = 0;
          component.field2_ = 0.0;
        }
    };

    TEST_F(PhysicalPlanGeneratorTest, PhysicalPlanGeneratorWorks) {

      auto execution_engine = ExecutionEngine::get_instance();
      execution_engine->register_system<TestSystem>();
      auto logic_plan = execution_engine->build_logic_plan();
      PhysicalPlan physical_plan = execution_engine->build_physical_plan(logic_plan);

      auto physical_root = physical_plan.m_roots[0];
      ASSERT_STREQ(physical_root->str().c_str(),"PhysicalMap(0)");
      auto physical_filter = physical_root->child(0);
      ASSERT_STREQ(physical_filter->str().c_str(),"PhysicalFilter()");
      auto physical_scan = physical_filter->child(0);
      ASSERT_STREQ(physical_scan->str().c_str(),"PhysicalScan(ComponentA)");

    }
    
} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
