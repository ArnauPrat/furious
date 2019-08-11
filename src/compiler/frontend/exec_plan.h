
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/types.h"
#include "../common/dyn_array.h"
#include "../common/refcount_ptr.h"

#include "fcc_context.h"

namespace furious 
{

struct fcc_system_t;
struct CascadingGather;
struct CrossJoin;
struct FccOperator;
struct Fetch;
template<typename T>
struct Filter;
struct PredicateFilter;
struct TagFilter;
struct ComponentFilter;
struct Foreach;
struct Gather;
struct Join;
struct LeftFilterJoin;
struct Scan;
class FccSubPlanVisitor;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct FccSubPlan
{
  FccOperator*  p_root;
};


/**
 * @brief Bootstraps an initial plan from an execution info
 *
 * @param exec_info The Exec info to bootstrap the plan from
 * @param subplan   The Subplan to initialize 
 *
 */
void 
init_subplan(const fcc_stmt_t* stmt, 
             FccSubPlan* subplan);

/**
 * \brief Destroys the given subplan
 *
 * \param subplan The subplan to release
 */
void 
release_subplan(FccSubPlan* subplan);



////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct FccExecPlanNode 
{
  DynArray<uint32_t>            m_parents;
  DynArray<uint32_t>            m_children;
};

struct FccExecPlan
{
  // Graph members
  DynArray<FccExecPlanNode>   m_nodes;
  DynArray<uint32_t>          m_roots;

  // Node attributes
  DynArray<const fcc_stmt_t*> p_stmts;
  DynArray<FccSubPlan>        m_subplans;
};

/**
 * \brief Inserts a fcc_stmt_t to the dependency graph
 *
 * \param exec_plan The exec_plan to insert the stmts to 
 * \param stmt      The fcc_stmt_t to add
 *
 */
void
insert(FccExecPlan* exec_plan, 
       const fcc_stmt_t* stmt);


/**
 * \brief Checks if the graph is acyclic
 *
 * \return Returns true if the graph is acyclyc
 */
bool 
is_acyclic(const FccExecPlan* exec_plan);

/**
 * \brief Gets a valid execution sequence from a given root node
 *
 * \param exec_plan The exec plan to get the valid sequence from
 * \param root The id of the root node to start the sequence from
 *
 * \return An array with the sequence of node ids.
 */
DynArray<uint32_t>
get_valid_exec_sequence(const FccExecPlan* exec_plan);

/**
 * \brief Creates an excution plan out of a sequence of fcc_stmt_t statements
 *
 * \param stmts[]   The array of fcc_stmt_t
 * \param num_stmts The number of matches in the array
 * \param exec_plan   The output execution plan
 *
 * \return Returns NO_ERROR if succeeds. The corresponding error code otherwise
 */
fcc_compilation_error_type_t
create_execplan(const fcc_stmt_t* stmts[], 
                uint32_t num_stmts,
                FccExecPlan** exec_plan);

/**
 * \brief Destroys the execution plan
 *
 * \param exec_plan The execution plan to destroy
 */
void
destroy_execplan(FccExecPlan* exec_plan);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


class FccSubPlanVisitor
{
public:
  void
  traverse(const FccSubPlan* subplan);

  virtual void 
  visit(const Foreach* foreach) = 0;

  virtual void 
  visit(const Scan* scan) = 0;

  virtual void
  visit(const Join* join) = 0;

  virtual void
  visit(const LeftFilterJoin* left_filter_join) = 0;

  virtual void
  visit(const CrossJoin* cross_join) = 0;

  virtual void
  visit(const Fetch* fetch) = 0;

  virtual void 
  visit(const TagFilter* tag_filter) = 0;

  virtual void
  visit(const ComponentFilter* component_filter) = 0;

  virtual void
  visit(const PredicateFilter* predicate_filter) = 0;

  virtual void
  visit(const Gather* gather) = 0;

  virtual void
  visit(const CascadingGather* casc_gather) = 0;
};

}  


#endif /* ifndef SYMBOL */
