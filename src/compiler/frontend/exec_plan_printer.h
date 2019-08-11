
#ifndef _FURIOUS_COMPILER_PRITER_H_
#define _FURIOUS_COMPILER_PRITER_H_

#include "exec_plan.h"
#include "../common/str_builder.h"
#include "../common/dyn_array.h"

namespace furious 
{
  
class ExecPlanPrinter : public FccSubPlanVisitor 
{
public:

  ExecPlanPrinter(bool add_comments = false);
  virtual ~ExecPlanPrinter();

  virtual void
  traverse(const FccSubPlan* plan);

  virtual void 
  visit(const Foreach* foreach) override;

  virtual void 
  visit(const Scan* scan) override;

  virtual void
  visit(const Join* join) override;

  virtual void
  visit(const LeftFilterJoin* join) override;

  virtual void
  visit(const CrossJoin* cross_join) override;

  virtual void
  visit(const Fetch* fetch) override;

  virtual void
  visit(const CascadingGather* join) override;

  virtual void 
  visit(const TagFilter* tag_filter) override;

  virtual void
  visit(const ComponentFilter* component_filter) override;

  virtual void
  visit(const PredicateFilter* predicate_filter) override;

  virtual void
  visit(const Gather* gather) override;

  void
  incr_level(bool siblings);

  void
  decr_level();

  void
  print(const std::string& str);

  int32_t               m_indents = 0;
  str_builder_t         m_str_builder;
  DynArray<char>        m_offsets;
};

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_PRITER_H_ */
