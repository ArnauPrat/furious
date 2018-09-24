

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
  ProduceVisitor produce{ss};
  for(const FccOperator* root : exec_plan->m_roots)
  {
    root->accept(&produce);
  }
  std::ofstream output_file(filename);
  output_file << ss.str();
  output_file.close();
}
  
} /* furious */ 
