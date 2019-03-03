
#ifndef _FURIOUS_COMPILER_CONSUME_VISITOR_H_
#define _FURIOUS_COMPILER_CONSUME_VISITOR_H_ value

#include "../frontend/execution_plan.h"

namespace furious 
{

class CodeGenContext;

/**
 * \brief Visitor that implements the consume operation for generating code
 */
class ConsumeVisitor : public FccExecPlanVisitor
{
public:

  ConsumeVisitor(CodeGenContext* context);
  virtual ~ConsumeVisitor() = default;

  virtual void 
  visit(const Foreach* foreach);

  virtual void 
  visit(const Scan* scan);

  virtual void
  visit(const Join* join);

  virtual void 
  visit(const TagFilter* tag_filter);

  virtual void
  visit(const ComponentFilter* component_filter);

  virtual void
  visit(const PredicateFilter* predicate_filter);

  virtual void
  visit(const Gather* gather);

  CodeGenContext* p_context;
};

} /* consume_visitor */ 

#endif
