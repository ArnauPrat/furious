

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

class CodeGenContext
{
public:
  CodeGenContext(std::stringstream& output);
  ~CodeGenContext();

  ProduceVisitor* p_producer;
  ConsumeVisitor* p_consumer;


  // Generic state
  std::string m_source;
  std::stringstream&        m_output_ss;
  std::vector<std::string>  m_types;
  const FccOperator*        p_caller;

  // Join related state
  std::vector<std::string>  m_left_types;
  std::string               m_hashtable_name;

};

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename);

  
} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CODEGEN_H_ */
