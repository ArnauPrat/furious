


#include "data_test.h"
#include "../data/execution_engine.h"
#include "../data/physical/physical_map.h"
#include "../data/physical/physical_scan.h"
#include "../data/static_system.h"

#include <gtest/gtest.h>

namespace furious {

class PhysicalMapTest : public DataTest {
};

class TestSystem : public StaticSystem<ComponentA> {
public:

  TestSystem(SystemId id ) : StaticSystem(id) {}
  virtual ~TestSystem() = default; 

  void run(ComponentA& component) override {
    component.field1_ = 4;
    component.field2_ = 4.0;
  }
};

TEST_F(PhysicalMapTest, PhysicalMapWorks) {

  for(uint32_t i = 0; i < 10000; ++i) {
    tableA_->insert(i,i*2,i*1.0);
  }

  ExecutionEngine* engine = ExecutionEngine::get_instance(); 
  engine->register_system<TestSystem>();

  IPhysicalOperatorSPtr physical_scanA( new PhysicalScan(tableA_));
  IPhysicalOperatorSPtr physical_map( new PhysicalMap(physical_scanA, engine->get_system(0)) );
  physical_map->open();
  BaseRow* row = physical_map->next();
  while(row != nullptr) {
    row = physical_map->next();
  }
  physical_map->close();

  for(auto it = tableA_->begin(); it != tableA_->end(); ++it) {
    ASSERT_EQ(static_cast<ComponentA*>(it->column(0))->field1_, 4);
    ASSERT_EQ(static_cast<ComponentA*>(it->column(0))->field2_, 4.0);
  }
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
