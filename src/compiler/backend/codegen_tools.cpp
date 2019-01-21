

#include "../common/common.h"
#include "consume_visitor.h"
#include "produce_visitor.h"

#include "codegen.h"
#include "codegen_tools.h"

#include <unordered_map>

#include <memory>

namespace furious {

CodeGenContext::CodeGenContext(FILE* fd) :
p_fd(fd)
{
  p_consumer = new ConsumeVisitor(this);
  p_producer = new ProduceVisitor(this);
}

CodeGenContext::~CodeGenContext()
{
  delete p_consumer;
  delete p_producer;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static 
std::unordered_map<const FccOperator*, std::unique_ptr<CodeGenContext>> contexts; 

void 
consume(FILE* fd,
        const FccOperator* op,
        const std::string& source,
        const std::vector<std::string>& types,
        const FccOperator* caller)
{
  auto it = contexts.find(op);
  if( it == contexts.end())
  {
    contexts[op] = std::make_unique<CodeGenContext>(fd);
  }  

  CodeGenContext* context = contexts[op].get();
  context->m_types = types;
  context->m_source = source;
  context->p_caller = caller;
  op->accept(context->p_consumer);
}

void 
produce(FILE* fd,
        const FccOperator* op)
{
  auto it = contexts.find(op);
  if( it == contexts.end())
  {
    contexts[op] = std::make_unique<CodeGenContext>(fd);
  }  
  CodeGenContext* context = contexts[op].get();
  op->accept(context->p_producer);
}

} /* furious */ 
