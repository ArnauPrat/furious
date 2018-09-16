
#include "../frontend/execution_plan.h"

namespace furious {

class ProduceVisitor;
  
class ConsumeVisitor : public FccExecPlanVisitor
{
public:

  ConsumeVisitor(std::ofstream& output_file);
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

private:
  std::ofstream& m_output_file;
  ProduceVisitor* p_produce;

};

} /* consume_visitor */ 
