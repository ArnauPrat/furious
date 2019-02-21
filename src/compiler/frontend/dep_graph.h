
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

  void 
  insert(const FccExecInfo* exec_info);

  DynArray<const FccExecInfo*>
  get_roots();

  DynArray<DGNode*>   p_nodes;
};
  
} /* furious
 */ 
#endif
