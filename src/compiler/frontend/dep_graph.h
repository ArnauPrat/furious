
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
  DGNode*             p_parent;
  DynArray<DGNode*>   p_children;
};

struct DependencyGraph
{
  DependencyGraph() = default;
  DependencyGraph(const DependencyGraph&) = delete;
  ~DependencyGraph();

  void 
  insert(const FccExecInfo* exec_info);

  DynArray<DGNode*>   p_roots;
};
  
} /* furious
 */ 
#endif
