

#include "physical_plan_generator.h"
#include "physical/physical_hash_join.h"
#include "physical/physical_scan.h"
#include "physical/physical_filter.h"
#include "physical/physical_map.h"
#include "database.h"
#include "logic/logic_join.h"
#include "logic/logic_map.h"
#include "logic/logic_scan.h"
#include "logic/logic_filter.h"
#include "execution_engine.h"

namespace furious {

void PhysicalPlanGenerator::visit(LogicJoin* logic_join) {
  PhysicalPlanGenerator gen;
  logic_join->p_left->accept(&gen);
  IPhysicalOperatorSPtr left = gen.get_result();
  logic_join->p_right->accept(&gen);
  IPhysicalOperatorSPtr right = gen.get_result();
  p_result = IPhysicalOperatorSPtr(new PhysicalHashJoin(left,right));
}

void PhysicalPlanGenerator::visit(LogicMap* logic_map) {
  PhysicalPlanGenerator gen;
  ExecutionEngine* execution_engine = ExecutionEngine::get_instance();
  logic_map->p_table->accept(&gen);
  p_result = IPhysicalOperatorSPtr(new PhysicalMap(gen.get_result(), execution_engine->get_system(logic_map->m_system)));
}

void PhysicalPlanGenerator::visit(LogicScan* logic_scan) {
  PhysicalPlanGenerator gen;
  Database* database = Database::get_instance();
  Table* table = database->find_table(logic_scan->m_table);
  p_result = IPhysicalOperatorSPtr( new PhysicalScan(table) );
}

void PhysicalPlanGenerator::visit(LogicFilter* logic_filter) {
  PhysicalPlanGenerator gen;
  logic_filter->p_table->accept(&gen);
  p_result = IPhysicalOperatorSPtr( new PhysicalFilter(gen.get_result()));
}

IPhysicalOperatorSPtr PhysicalPlanGenerator::get_result() {
  return p_result;
}

} /* furious */ 
