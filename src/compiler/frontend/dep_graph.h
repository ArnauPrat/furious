
#ifndef _FURIOUS_DEP_GRAPH_H_
#define _FURIOUS_DEP_GRAPH_H_

#include "../../common/dyn_array.h"

namespace furious
{

struct FccExecInfo;

struct DGNode
{
  DGNode(const FccExecInfo* info);
  ~DGNode();

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

  DynArray<DGNode*>   p_nodes;
};
  
} /* furious
 */ 
#endif
