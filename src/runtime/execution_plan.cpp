
#include "execution_plan.h"

namespace furious {

ExecutionPlan::~ExecutionPlan() {
  for(auto query : m_queries) {
    delete query;
  }
}

Operator::Operator(OperatorType type) : 
  m_type(type) {

}

Scan::Scan(const std::string& table_name) : 
  Operator(OperatorType::E_SCAN), 
  m_table_name(table_name) {

}

Join::Join(Operator* left, Operator* right) :
  Operator(OperatorType::E_JOIN),
  p_left(left),
  p_right(right) {

  }

Join::~Join() {
  if(p_left != nullptr) {
    delete p_left;
  }

  if(p_right != nullptr) {
    delete p_right;
  }
}

Filter::Filter(const std::string& tag_name, Operator* child) : 
  Operator(OperatorType::E_FILTER),
  m_tag(tag_name),
  p_child(child) {

  }

Filter::~Filter() {
  if(p_child != nullptr) {
    delete p_child;
  }
}

Foreach::Foreach(const std::string& system_name, Operator* child) :
  Operator(OperatorType::E_FOREACH), 
  m_system_name(system_name), 
  p_child(child) {

  }

Foreach::~Foreach() {
  if(p_child != nullptr) {
    delete p_child;
  }
}


ExecutionPlan* create_execution_plan( const std::vector<SystemExecInfo>& systems ) {

  ExecutionPlan* plan = new ExecutionPlan();
  for(auto& system_info : systems) {
    System* system = system_info.p_system;
    std::vector<Scan*> scans;
    for(auto& com_info : system->components()) {
      scans.push_back(new Scan(com_info.m_name));
    }
    Operator* root = *scans.begin();
    for(int i = 1; i < static_cast<int32_t>(scans.size()); ++i) {
      Join* join = new Join(root, scans[i]);
      root = join;
    }

    for(auto& tag : system_info.m_tags) {
      Filter* filter = new Filter(tag, root);
      root = filter;
    }

    root = new Foreach(system->name(), root);
    plan->m_queries.push_back(root);
  }
  return plan;

}

void destroy_execution_plan( ExecutionPlan* exec_plan ) {
  delete exec_plan;
}
  
} /* furious */ 
