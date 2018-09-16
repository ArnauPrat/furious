

#include "../frontend/execution_plan.h"
#include "codegen.h"
#include "consume_visitor.h"
#include "produce_visitor.h"


#include <sstream>
#include <fstream>

namespace furious {

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename)
{
  std::stringstream ss;
  ConsumeVisitor* consume = new ConsumeVisitor(ss);
  ProduceVisitor* produce = new ProduceVisitor(ss);
  for(const FccOperator* root : exec_plan->m_roots)
  {
    root->accept(produce);
  }
  delete consume;
  delete produce;
  std::ofstream output_file(filename);
  output_file.close();
}
  
} /* furious */ 
