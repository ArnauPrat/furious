
#ifndef _FURIOUS_DEP_GRAPH_H_
#define _FURIOUS_DEP_GRAPH_H_

#include "../../common/dyn_array.h"

namespace furious
{

struct FccExecInfo;

struct DGNode
{
  DGNode(uint32_t id,
         const FccExecInfo* info);
  ~DGNode();

  uint32_t            m_id;
  const FccExecInfo*  p_info;
  DynArray<DGNode*>   p_parents;
  DynArray<DGNode*>   p_children;
};

struct DependencyGraph
{
  DependencyGraph() = default;
  DependencyGraph(const DependencyGraph&) = delete;
  ~DependencyGraph();

  /**
   * \brief Inserts a FccExecInfo to the dependency graph
   *
   * \param exec_info The FccExecInfo to add
   */
  void 
  insert(const FccExecInfo* exec_info);

  /**
   * \brief Gets the roots of the dependency graph (FccExecInfos without
   * parents)
   *
   * \return Returns a dynamic array with the roots.
   */
  DynArray<const FccExecInfo*>
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
  DynArray<const FccExecInfo*>
  get_valid_exec_sequence();


  DynArray<DGNode*>   p_nodes;
};
  
} /* furious
 */ 
#endif
