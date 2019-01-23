
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include <vector>
#include <string>

#include <stdio.h>

namespace furious 
{

class FccOperator;
class ProduceVisitor;
class ConsumeVisitor;

/**
 * \brief This class represents the context of the code generation process at a
 * given code generation step. Since the code generation is a recursive
 * procedure implemented through the visitors, we need to store the "state" of
 * the generation process at a given step produce/consume step.
 */
class CodeGenContext
{
public:
  CodeGenContext(FILE* fd);
  ~CodeGenContext();

  ProduceVisitor* p_producer;
  ConsumeVisitor* p_consumer;

  // Generic state
  std::string m_source;
  FILE*                     p_fd;
  std::vector<std::string>  m_types;
  const FccOperator*        p_caller;

  // Join related state
  std::vector<std::string>  m_left_types;
  std::string               m_join_id;

};

void 
consume(FILE* fd,
        const FccOperator* op,
        const std::string& source,
        const std::vector<std::string>& types,
        const FccOperator* caller);

void 
produce(FILE* fd,
        const FccOperator* op);

} /* furious */ 

#endif
