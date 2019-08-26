
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/types.h"
#include "../common/dyn_array.h"

#include "fcc_context.h"

namespace furious 
{

struct fcc_system_t;
struct fcc_subplan_t;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct fcc_exec_plan_node_t 
{
  DynArray<uint32_t>            m_parents;
  DynArray<uint32_t>            m_children;
};

struct fcc_exec_plan_t 
{
  // Graph members
  DynArray<fcc_exec_plan_node_t>   m_nodes;
  DynArray<uint32_t>            m_roots;

  // Node attributes
  DynArray<const fcc_stmt_t*> p_stmts;
  DynArray<fcc_subplan_t*>     m_subplans;
};

/**
 * \brief Inserts a fcc_stmt_t to the dependency graph
 *
 * \param exec_plan The exec_plan to insert the stmts to 
 * \param stmt      The fcc_stmt_t to add
 *
 */
void
insert(fcc_exec_plan_t* exec_plan, 
       const fcc_stmt_t* stmt);


/**
 * \brief Checks if the graph is acyclic
 *
 * \return Returns true if the graph is acyclyc
 */
bool 
is_acyclic(const fcc_exec_plan_t* exec_plan);

/**
 * \brief Gets a valid execution sequence from a given root node
 *
 * \param exec_plan The exec plan to get the valid sequence from
 * \param root The id of the root node to start the sequence from
 *
 * \return An array with the sequence of node ids.
 */
DynArray<uint32_t>
get_valid_exec_sequence(const fcc_exec_plan_t* exec_plan);

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
                fcc_exec_plan_t** exec_plan);

/**
 * \brief Destroys the execution plan
 *
 * \param exec_plan The execution plan to destroy
 */
void
destroy_execplan(fcc_exec_plan_t* exec_plan);

}  


#endif /* ifndef SYMBOL */
