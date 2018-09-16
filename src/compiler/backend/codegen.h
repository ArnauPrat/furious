

#ifndef _FURIOUS_COMPILER_CODEGEN_H_
#define _FURIOUS_COMPILER_CODEGEN_H_ value

#include <string>

namespace furious {

struct FccExecPlan;

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename);

  
} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CODEGEN_H_ */
