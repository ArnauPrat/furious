

#include "../common/types.h"
#include "execution_engine.h"
#include "logic/logic_filter.h"
#include "logic/logic_join.h"
#include "logic/logic_map.h"
#include "logic/logic_scan.h"
#include "physical/physical_filter.h"
#include "physical/physical_hash_join.h"
#include "physical/physical_plan.h"
#include "physical/physical_scan.h"
#include "physical_plan_generator.h"

#include <cstddef>

namespace furious {

ExecutionEngine::~ExecutionEngine() {

  for (auto& iter : m_systems) {
    delete iter.second;
  }
  m_systems.clear();

}


ExecutionEngine* ExecutionEngine::get_instance() {
  static ExecutionEngine instance;
  return &instance;
}

void ExecutionEngine::run_systems() const {
  auto logic_plan = build_logic_plan();
  PhysicalPlan physical_plan = build_physical_plan(logic_plan);
  execute_physical_plan(physical_plan);
}

void ExecutionEngine::execute_physical_plan(const PhysicalPlan& physical_plan ) const {
  for(auto root : physical_plan.m_roots) {
    BaseRow* next_row = root->next();
    while(next_row != nullptr) {
      next_row = root->next();
    }
  }
}

System* ExecutionEngine::get_system(SystemId system) {
  return (*m_systems.find(system)).second;
}

LogicPlan ExecutionEngine::build_logic_plan() const {
  LogicPlan logic_plan;
  for(auto system : m_systems ) {
    if(system.second->components().size() == 1) { // Case when join is not required
      ILogicPlanNodeSPtr logic_scan = MakeLogicPlanNodeSPtr<LogicScan>(*system.second->components().begin());
      ILogicPlanNodeSPtr logic_filter = MakeLogicPlanNodeSPtr<LogicFilter>(logic_scan);
      ILogicPlanNodeSPtr logic_map = MakeLogicPlanNodeSPtr<LogicMap>(system.first,logic_filter);
      logic_plan.m_roots.push_back(logic_map);
    } else { // Case when we have at least one join (2-component case)
      std::vector<std::string> components = system.second->components();
      std::string first_component = components[0];
      std::string second_component = components[1];
      ILogicPlanNodeSPtr logic_scan_first = MakeLogicPlanNodeSPtr<LogicScan>(first_component);
      ILogicPlanNodeSPtr logic_filter_first = MakeLogicPlanNodeSPtr<LogicFilter>(logic_scan_first);
      ILogicPlanNodeSPtr logic_scan_second = MakeLogicPlanNodeSPtr<LogicScan>(second_component);
      ILogicPlanNodeSPtr logic_filter_second = MakeLogicPlanNodeSPtr<LogicFilter>(logic_scan_second);
      ILogicPlanNodeSPtr previous_join = MakeLogicPlanNodeSPtr<LogicJoin>(logic_filter_first, logic_filter_second);
      for (size_t i = 2; i < components.size(); ++i ) {
        std::string next_component = components[i];
        ILogicPlanNodeSPtr logic_scan_next = MakeLogicPlanNodeSPtr<LogicScan>(next_component);
        ILogicPlanNodeSPtr logic_filter_next = MakeLogicPlanNodeSPtr<LogicFilter>(logic_scan_next);
        ILogicPlanNodeSPtr next_join = MakeLogicPlanNodeSPtr<LogicJoin>(previous_join,logic_filter_next);
        previous_join = next_join;
      }
      ILogicPlanNodeSPtr logic_map = MakeLogicPlanNodeSPtr<LogicMap>(system.first, previous_join);
      logic_plan.m_roots.push_back(logic_map);
    }
  }
  return logic_plan;
}

PhysicalPlan  ExecutionEngine::build_physical_plan( const LogicPlan& logic_plan) const {
  PhysicalPlanGenerator gen;
  PhysicalPlan physical_plan;
  for(ILogicPlanNodeSPtr node : logic_plan.m_roots) {
    node->accept(&gen);
    physical_plan.m_roots.push_back(gen.get_result());
  }
  return physical_plan;
}

void ExecutionEngine::clear() {
  m_systems.clear();
}
} /* furious */ 
