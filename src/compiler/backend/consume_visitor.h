
#include "../frontend/execution_plan.h"

namespace furious {

class ProduceVisitor;
  
class ConsumeVisitor : public FccExecPlanVisitor
{
public:

  ConsumeVisitor(std::stringstream& output_ss,
                 const std::string& source,
                 const std::vector<std::string>& m_types,
                 const FccOperator* caller);
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

  const std::string m_source;
private:
  std::stringstream&        m_output_ss;
  std::vector<std::string>  m_types;
  const FccOperator*        p_caller;

};

} /* consume_visitor */ 
