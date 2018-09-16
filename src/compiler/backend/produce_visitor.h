
#ifndef _FURIOUS_COMPILER_PRODUCE_VISITOR_H_
#define _FURIOUS_COMPILER_PRODUCE_VISITOR_H_ value

#include "frontend/execution_plan.h"


namespace furious {

class ConsumeVisitor;

class ProduceVisitor : public FccExecPlanVisitor
{
public:

  ProduceVisitor(std::stringstream& output_stream);

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

private:
  std::stringstream&  m_output_ss;
};
  
} /* produce_visitor */ 

#endif /* ifndef _FURIOUS_COMPILER_PRODUCE_VISITOR */
