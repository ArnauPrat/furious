
#ifndef _FURIOUS_DEP_GRAPH_H_
#define _FURIOUS_DEP_GRAPH_H_

#include "../../common/dyn_array.h"

namespace furious
{

struct FccMatch;

struct DGNode
{
  DGNode(uint32_t id,
         const FccMatch* info);
  ~DGNode();

  uint32_t            m_id;
  const FccMatch*     p_match;
  DynArray<DGNode*>   p_parents;
  DynArray<DGNode*>   p_children;
};

struct DependencyGraph
{
  DependencyGraph() = default;
  DependencyGraph(const DependencyGraph&) = delete;
  ~DependencyGraph();

  /**
   * \brief Inserts a FccMatch to the dependency graph
   *
   * \param exec_info The FccMatch to add
   */
  bool 
  insert(const FccMatch* exec_info);

  /**
   * \brief Gets the roots of the dependency graph (FccMatchs without
   * parents)
   *
   * \return Returns a dynamic array with the roots.
   */
  DynArray<const FccMatch*>
  get_roots();

  /**
   * \brief Checks if the graph is acyclic
   *
   * \return Returns true if the graph is acyclyc
   */
  bool 
  is_acyclic();

  /**
   * \brief Gets a valid sequence of execution info given the dependencies
   * between them.
   *
   * \return Returns a dynamic array with the valid sequence of execution info
   */
  DynArray<const FccMatch*>
  get_valid_exec_sequence();


  DynArray<DGNode*>   p_nodes;
};
  
} /* furious
 */ 
#endif
