

#include "temp_buffer.h"
#include "volcano.h"
#include "../../execution_plan.h"
#include "../../../data/table.h"

#define VOLCANO_BLOCK_SIZE TABLE_BLOCK_SIZE

namespace furious {

static int32_t max_components(Operator* root) {
  switch(root->m_type) {
  case OperatorType::E_FILTER:
    {
      Filter* filter = static_cast<Filter*>(root);
      return max_components(filter->p_child);
    }
  case OperatorType::E_JOIN:
    {
      Join* join = static_cast<Join*>(root);
      return max_components(join->p_left) + max_components(join->p_right);
    }
  case OperatorType::E_FOREACH:
    {
      Foreach* foreach = static_cast<Foreach*>(root);
      return max_components(foreach->p_child);
    }
  case OperatorType::E_SCAN:
    {
      return 1;
    }
  }
  return 1;
}

Volcano::Volcano() :
p_exec_plan{nullptr} {

}

Volcano::~Volcano() {
  reset();
}

void Volcano::compile(Workload* workload,
                      Database* database) {
  reset();
  p_exec_plan = create_execution_plan(workload, database);

  int32_t max_components_size = 0;
  for(auto& query: p_exec_plan->m_queries) {
    max_components_size = std::max(max_components(query), max_components_size);
  }

  p_temp_buffer = new TempBuffer(VOLCANO_BLOCK_SIZE, max_components_size);

}

void Volcano::run(float delta) {

}

void Volcano::reset() {
  if(p_exec_plan != nullptr) {
    destroy_execution_plan(p_exec_plan);
    p_exec_plan = nullptr;
  }
}

  
} /* furious */ 
