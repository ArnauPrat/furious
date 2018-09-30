
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include <sstream>
#include <vector>
#include <string>

namespace furious 
{

class FccOperator;
class ProduceVisitor;
class ConsumeVisitor;

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
  std::string               m_join_id;

};

void 
consume(std::stringstream& output,
        const FccOperator* op,
        const std::string& source,
        const std::vector<std::string>& types,
        const FccOperator* caller);

void 
produce(std::stringstream& output,
        const FccOperator* op);

} /* furious */ 

#endif
