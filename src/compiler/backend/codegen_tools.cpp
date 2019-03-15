

#include "../common/types.h"
#include "../common/dyn_array.h"
#include "consume_visitor.h"
#include "produce_visitor.h"

#include "codegen.h"
#include "codegen_tools.h"

namespace furious 
{

extern CodeGenRegistry* p_registry;

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


CodeGenRegistry::CodeGenRegistry()
{
}

CodeGenRegistry::~CodeGenRegistry()
{
  for(uint32_t i = 0; i < m_contexts.size(); ++i)
  {
    delete m_contexts[i].p_context;
  }
}

CodeGenContext* 
CodeGenRegistry::find_or_create(const FccOperator* op, FILE* fd)
{
  for(uint32_t i = 0; i < m_contexts.size(); ++i)
  {
    if(m_contexts[i].p_operator == op)
    {
      return m_contexts[i].p_context;
    }
  }

  Entry entry = {op,new CodeGenContext(fd)};
  m_contexts.append(entry);
  return entry.p_context;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void 
consume(FILE* fd,
        const FccOperator* op,
        const std::string& source,
        const FccOperator* caller)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  context->m_source = source;
  context->p_caller = caller;
  op->accept(context->p_consumer);
}

void 
produce(FILE* fd,
        const FccOperator* op)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  op->accept(context->p_producer);
}

} /* furious */ 
