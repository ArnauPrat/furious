

#include "dep_graph.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious
{

DGNode::DGNode(const FccExecInfo* info) :
p_info(info)
{
}

DGNode::~DGNode()
{
}

DependencyGraph::~DependencyGraph()
{
  for(uint32_t i = 0; i < p_nodes.size(); ++i)
  {
    delete p_nodes[i];
  }
}

bool
is_dependent(DGNode* node_a, DGNode* node_b)
{
  DynArray<QualType> write_types_b;
  uint32_t size_b = node_b->p_info->m_basic_component_types.size(); 
  for(uint32_t i = 0; i < size_b; ++i)
  {
    const QualType& type = node_b->p_info->m_basic_component_types[i];
    if(get_access_mode(type) == AccessMode::E_WRITE)
    {
      write_types_b.append(type);
    }
  }

  uint32_t size_a = node_a->p_info->m_basic_component_types.size();
  for(uint32_t i = 0; i < size_a; ++i)
  {
    const QualType& type_a = node_a->p_info->m_basic_component_types[i];
    std::string name_a = get_type_name(type_a);
    for(uint32_t i = 0; i < size_b; ++i)
    {
      const QualType& type_b = node_a->p_info->m_basic_component_types[i];
      std::string name_b = get_type_name(type_b);
      if(name_a == name_b)
      {
        return true;
      }
    }
  }
  return false;
}

void 
DependencyGraph::insert(const FccExecInfo* exec_info)
{
  DGNode* node = new DGNode(exec_info);
  for(uint32_t i = 0; p_nodes.size(); ++i)
  {
    if(is_dependent(node,p_nodes[i]))
    {
      p_nodes[i]->p_children.append(node);
      node->p_parents.append(p_nodes[i]);
    }

    if(is_dependent(p_nodes[i], node))
    {
      node->p_children.append(p_nodes[i]);
      p_nodes[i]->p_parents.append(node);
    }
  }
  p_nodes.append(node);
}

DynArray<const FccExecInfo*>
DependencyGraph::get_roots()
{
  DynArray<const FccExecInfo*> ret;
  uint32_t size = p_nodes.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    if(p_nodes[i]->p_parents.size() == 0)
    {
      ret.append(p_nodes[i]->p_info);
    }
  }
  return ret;
}

} /* furious
 */ 
