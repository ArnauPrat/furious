
#ifndef _FURIOUS_COMPILER_PRITER_H_
#define _FURIOUS_COMPILER_PRITER_H_

#include "execution_plan.h"

#include <sstream>

namespace furious {
  
class ExecPlanPrinter : public FccExecPlanVisitor 
{
public:

  ExecPlanPrinter(bool add_comments = false);
  virtual ~ExecPlanPrinter() = default;

  virtual void
  traverse(const FccExecPlan* plan);

  virtual void 
  visit(const Foreach* foreach) override;

  virtual void 
  visit(const Scan* scan) override;

  virtual void
  visit(const Join* join) override;

  virtual void 
  visit(const TagFilter* tag_filter) override;

  virtual void
  visit(const ComponentFilter* component_filter) override;

  virtual void
  visit(const PredicateFilter* predicate_filter) override;

  void
  incr_level(bool siblings);

  void
  decr_level();

  void
  print(const std::string& str);

  int32_t m_indents = 0;
  std::stringstream   m_string_builder;
  std::vector<char>   m_offsets;
};

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_PRITER_H_ */
