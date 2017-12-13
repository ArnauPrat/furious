

#include "data_test.h"
#include "../data/static_system.h"
#include "../data/execution_engine.h"
#include "../data/logic/logic_plan.h"

#include <gtest/gtest.h> 
#include <iostream>

namespace furious {

struct ComponentC {
  uint32_t field1_;
  double field2_;
  ComponentC(uint32_t field1, double field2) : field1_(field1), field2_(field2) {}

  static std::string name() { return "ComponentC"; }

};

class ExecutionEngineTest : public DataTest {
};

class SystemTest : public DataTest {
};

class TestSystemA : public StaticSystem<ComponentA> {
public:
  TestSystemA(SystemId id) : StaticSystem(id) {}
  virtual ~TestSystemA() = default;

  void run(ComponentA& component) override {
    component.field1_ *= 2;
    component.field2_ *= 2.0;
  }
};

class TestSystemB : public StaticSystem<ComponentB> {
public:

  TestSystemB(SystemId id) : StaticSystem(id) {}
  virtual ~TestSystemB() = default;

  void run(ComponentB& component) override {
    component.field1_ *= 2;
    component.field2_ *= 2.0;
  }
};

class TestSystemAB : public StaticSystem<ComponentA, ComponentB> {
public:

  TestSystemAB(SystemId id) : StaticSystem(id) {}
  virtual ~TestSystemAB() = default;

  void run(ComponentA& componentA, ComponentB& componentB) override {
  }
};

class TestSystemABC : public StaticSystem<ComponentA, ComponentB, ComponentC> {
public:

  TestSystemABC(SystemId id) : StaticSystem(id) {}
  virtual ~TestSystemABC() = default;

  void run(ComponentA& componentA, ComponentB& componentB, ComponentC& componentC) override {
  }
};

TEST_F(SystemTest, LogicPlan) {

  Database* database = Database::get_instance();
  database->create_table<ComponentC>();

  ExecutionEngine* execution_engine = ExecutionEngine::get_instance();
  execution_engine->clear();

  execution_engine->register_system<TestSystemA>();
  execution_engine->register_system<TestSystemB>();
  execution_engine->register_system<TestSystemAB>();
  execution_engine->register_system<TestSystemABC>();

  
  LogicPlan logic_plan = execution_engine->build_logic_plan(); 
  ILogicPlanNodeSPtr rootA = logic_plan.m_roots[0];
  ASSERT_STREQ(rootA->str().c_str(), "LogicMap(0)");
  ILogicPlanNodeSPtr filterA = rootA->child(0);
  ASSERT_STREQ(filterA->str().c_str(), "LogicFilter()");
  ILogicPlanNodeSPtr scanA = filterA->child(0);
  ASSERT_STREQ(scanA->str().c_str(), "LogicScan(ComponentA)");

  ILogicPlanNodeSPtr rootB = logic_plan.m_roots[1];
  ASSERT_STREQ(rootB->str().c_str(), "LogicMap(1)");
  ILogicPlanNodeSPtr filterB = rootB->child(0);
  ASSERT_STREQ(filterB->str().c_str(), "LogicFilter()");
  ILogicPlanNodeSPtr scanB = filterB->child(0);
  ASSERT_STREQ(scanB->str().c_str(), "LogicScan(ComponentB)");

  ILogicPlanNodeSPtr rootAB = logic_plan.m_roots[2];
  ASSERT_STREQ(rootAB->str().c_str(), "LogicMap(2)");
  ILogicPlanNodeSPtr joinAB = rootAB->child(0);
  ASSERT_STREQ(joinAB->str().c_str(), "LogicJoin()");

  ILogicPlanNodeSPtr filterAB_left = joinAB->child(0);
  ASSERT_STREQ(filterAB_left->str().c_str(), "LogicFilter()");
  ILogicPlanNodeSPtr scanAB_left = filterAB_left->child(0);
  ASSERT_STREQ(scanAB_left->str().c_str(), "LogicScan(ComponentA)");

  ILogicPlanNodeSPtr filterAB_right = joinAB->child(1);
  ASSERT_STREQ(filterAB_right->str().c_str(), "LogicFilter()");
  ILogicPlanNodeSPtr scanAB_right = filterAB_right->child(0);
  ASSERT_STREQ(scanAB_right->str().c_str(), "LogicScan(ComponentB)");

  ILogicPlanNodeSPtr rootABC = logic_plan.m_roots[3];
  ASSERT_STREQ(rootABC->str().c_str(), "LogicMap(3)");
  ILogicPlanNodeSPtr joinAB_C = rootABC->child(0);
  ASSERT_STREQ(joinAB_C->str().c_str(), "LogicJoin()");
  ILogicPlanNodeSPtr filterC = joinAB_C->child(1);
  ASSERT_STREQ(filterC->str().c_str(), "LogicFilter()");
  ILogicPlanNodeSPtr scanC = filterC->child(0);
  ASSERT_STREQ(scanC->str().c_str(), "LogicScan(ComponentC)");
  joinAB = joinAB_C->child(0);
  ASSERT_STREQ(joinAB->str().c_str(), "LogicJoin()");

  filterAB_left = joinAB->child(0);
  ASSERT_STREQ(filterAB_left->str().c_str(), "LogicFilter()");
  scanAB_left = filterAB_left->child(0);
  ASSERT_STREQ(scanAB_left->str().c_str(), "LogicScan(ComponentA)");

  filterAB_right = joinAB->child(1);
  ASSERT_STREQ(filterAB_right->str().c_str(), "LogicFilter()");
  scanAB_right = filterAB_right->child(0);
  ASSERT_STREQ(scanAB_right->str().c_str(), "LogicScan(ComponentB)");
}

TEST_F(ExecutionEngineTest, ExecutionEngineWorks) {

  for(uint32_t i = 0; i < 1; ++i) {
    tableA_->insert(i,4,4.0);
  }

  for(uint32_t i = 0; i < 1; ++i) {
    tableB_->insert(i,16,16.0);
  }

  ExecutionEngine* execution_engine = ExecutionEngine::get_instance();
  execution_engine->clear();

  execution_engine->register_system<TestSystemA>();
  execution_engine->register_system<TestSystemB>();
  execution_engine->register_system<TestSystemAB>();
  execution_engine->run_systems();

  for(auto iter_tableA = tableA_->begin();
      iter_tableA != tableA_->end();
      ++iter_tableA) {
    ComponentA* component = static_cast<ComponentA*>(iter_tableA->column(0));
    ASSERT_EQ(component->field1_,8);
    ASSERT_EQ(component->field2_,8.0);
  }

  for(auto iter_tableB = tableB_->begin();
      iter_tableB != tableB_->end();
      ++iter_tableB) {
    ComponentB* component = static_cast<ComponentB*>(iter_tableB->column(0));
    ASSERT_EQ(component->field1_,32);
    ASSERT_EQ(component->field2_,32.0);
  }
}
} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
