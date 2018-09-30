

#ifndef _FURIOUS_COMPILER_CODEGEN_H_
#define _FURIOUS_COMPILER_CODEGEN_H_ value

#include <sstream>
#include <string>
#include <vector>

namespace furious {

struct FccExecPlan;
class ProduceVisitor;
class ConsumeVisitor;
class FccOperator;


void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename);

  
} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CODEGEN_H_ */
