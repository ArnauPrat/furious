
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include "../common/dyn_array.h"

#include <string>
#include <stdio.h>

namespace furious 
{

struct FccContext;
class FccOperator;
class ProduceVisitor;
class ConsumeVisitor;

/**
 * \brief This class represents the context of the code generation process at a
 * given code generation step. Since the code generation is a recursive
 * procedure implemented through the visitors, we need to store the "state" of
 * the generation process at a given step produce/consume step.
 */
struct CodeGenContext
{
  CodeGenContext(FILE* fd);
  ~CodeGenContext();

  ProduceVisitor* p_producer;
  ConsumeVisitor* p_consumer;

  std::string m_source;
  FILE*                   p_fd;
  const FccOperator*      p_caller;
  FccContext*             p_fcc_context;
};

struct CodeGenRegistry
{
  struct Entry
  {
    const FccOperator*  p_operator;
    CodeGenContext*     p_context;
  };

  CodeGenContext* 
  find_or_create(const FccOperator*, FILE* fd);

  CodeGenRegistry();
  ~CodeGenRegistry();

  DynArray<Entry> m_contexts; 
};


void 
consume(FccContext* fcc_context,
        FILE* fd,
        const FccOperator* op,
        const std::string& source,
        const FccOperator* caller);

void 
produce(FccContext* fcc_context,
        FILE* fd,
        const FccOperator* op);

} /* furious */ 

#endif
