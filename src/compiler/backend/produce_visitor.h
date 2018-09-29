
#ifndef _FURIOUS_COMPILER_PRODUCE_VISITOR_H_
#define _FURIOUS_COMPILER_PRODUCE_VISITOR_H_ value

#include "frontend/execution_plan.h"
#include <sstream>


namespace furious {

class CodeGenContext;

class ProduceVisitor : public FccExecPlanVisitor
{
public:

  ProduceVisitor(CodeGenContext* p_context);

  virtual ~ProduceVisitor() = default;

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

  CodeGenContext* p_context;
};
  
} /* produce_visitor */ 

#endif /* ifndef _FURIOUS_COMPILER_PRODUCE_VISITOR */
