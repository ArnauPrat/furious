

#include "dep_graph.h"

namespace furious
{

DGNode::DGNode(const FccExecInfo* info) :
p_info(info)
{
}

DGNode::~DGNode()
{
  uint32_t num_children = p_children.size();
  for(uint32_t i = 0; i < num_children; ++i)
  {
    delete p_children[i];
  }
}

DependencyGraph::~DependencyGraph()
{
  for(uint32_t i = 0; i < p_roots.size(); ++i)
  {
    delete p_roots[i];
  }
}


bool
find_dependent(DGNode* root, 
               DGNode* node)
{

  return false;
}

void 
DependencyGraph::insert(const FccExecInfo* exec_info)
{
  DGNode* node = new DGNode(exec_info);
  for(uint32_t i = 0; p_roots.size(); ++i)
  {
    if(find_dependent(p_roots[i], node))
    {
      return;
    }
  }
  
  // we did not find a dependent node
  p_roots.append(node);
}
  
} /* furious
 */ 
