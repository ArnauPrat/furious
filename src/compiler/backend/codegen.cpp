

#include "../frontend/execution_plan.h"
#include "codegen.h"
#include "consume_visitor.h"
#include "produce_visitor.h"


#include <fstream>

namespace furious {


ConsumeVisitor* consume;
ProduceVisitor* produce;

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename)
{
  std::ofstream output_file(filename);
  consume = new ConsumeVisitor(output_file);
  produce = new ProduceVisitor(output_file);
  for(const FccOperator* root : exec_plan->m_roots)
  {
    root->accept(produce);
  }
  delete consume;
  delete produce;
  output_file.close();
}
  
} /* furious */ 
