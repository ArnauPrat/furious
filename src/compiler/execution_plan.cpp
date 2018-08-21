
#include "execution_plan.h"

namespace furious {

Operator::Operator(OperatorType type) : 
  m_type(type) {

}

Scan::Scan(int32_t table_id) : 
  Operator(OperatorType::E_SCAN), 
  m_table_id(table_id) {

}

Join::Join(Operator* left, 
           Operator* right) :
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

Filter::Filter( Operator* child, 
                int32_t func_id) : 
  Operator(OperatorType::E_FILTER),
  m_func_id(func_id),
  p_child(child) {

  }

Filter::~Filter() {
  if(p_child != nullptr) {
    delete p_child;
  }
}

Foreach::Foreach(Operator* child,
                 const std::vector<int32_t>& funcs) :
  Operator(OperatorType::E_FOREACH), 
  m_funcs{funcs}, 
  p_child{child} {

  }

Foreach::~Foreach() {
  if(p_child != nullptr) {
    delete p_child;
  }
}

} /* furious */ 
