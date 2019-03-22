

#include "dep_graph.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious
{

DGNode::DGNode(uint32_t id, 
               const FccMatch* match) :
m_id(id),
p_match(match)
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
  uint32_t size_b = node_b->p_match->m_system.m_component_types.size(); 
  for(uint32_t i = 0; i < size_b; ++i)
  {
    const QualType& type = node_b->p_match->m_system.m_component_types[i];
    if(get_access_mode(type) == FccAccessMode::E_READ_WRITE)
    {
      write_types_b.append(type);
    }
  }

  uint32_t size_a = node_a->p_match->m_system.m_component_types.size();
  for(uint32_t i = 0; i < size_a; ++i)
  {
    const QualType& type_a = node_a->p_match->m_system.m_component_types[i];
    std::string name_a = get_type_name(type_a);
    FccAccessMode access_a = get_access_mode(type_a);

    size_b = write_types_b.size();
    for(uint32_t ii = 0; ii < size_b; ++ii)
    {
      const QualType& type_b = write_types_b[ii];
      std::string name_b = get_type_name(type_b);
      if(name_a == name_b && 
         !(access_a == FccAccessMode::E_READ_WRITE && 
           node_a->p_match->m_system.m_is_outputwriteonly &&
           !node_b->p_match->m_system.m_is_outputwriteonly 
         ))
      {
        return true;
      }
    }
  }
  return false;
}

void 
DependencyGraph::insert(const FccMatch* exec_info)
{
  DGNode* node = new DGNode(p_nodes.size(),
                            exec_info);
  for(uint32_t i = 0; i < p_nodes.size(); ++i)
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

DynArray<const FccMatch*>
DependencyGraph::get_roots()
{
  DynArray<const FccMatch*> ret;
  uint32_t size = p_nodes.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    if(p_nodes[i]->p_parents.size() == 0)
    {
      ret.append(p_nodes[i]->p_match);
    }
  }
  return ret;
}

void
dfs(DependencyGraph* graph, 
    DynArray<bool>* visited,
    DynArray<uint32_t>* departure,
    uint32_t* time,
    uint32_t node)
{
  (*visited)[node] = true;
  DGNode* dg_node = graph->p_nodes[node];
  uint32_t size = dg_node->p_children.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    DGNode* child = dg_node->p_children[i];
    if(!(*visited)[child->m_id])
    {
      dfs(graph,
          visited,
          departure,
          time,
          child->m_id);

    }
  }

  (*departure)[node] = *time;
  ++(*time);
}

bool
DependencyGraph::is_acyclic()
{
  DynArray<bool> visited;
  for(uint32_t i = 0; i < p_nodes.size(); ++i)
  {
    visited.append(false);
  }
  DynArray<uint32_t> departure;

  for(uint32_t i = 0; i < p_nodes.size(); ++i)
  {
    departure.append(0);
  }

  uint32_t time=0;
  for(uint32_t i = 0; i < p_nodes.size(); ++i)
  {
    if(!visited[i] && p_nodes[i]->p_parents.size() == 0)
    {
      dfs(this, 
          &visited, 
          &departure, 
          &time, 
          i);
    }
  }

  for(uint32_t i = 0; i < p_nodes.size(); ++i)
  {
    DGNode* node = p_nodes[i];
    for(uint32_t ii = 0; ii < node->p_children.size(); ++ii)
    {
      DGNode* child = node->p_children[ii];
      if(departure[node->m_id] <= departure[child->m_id])
      {
        return false;
      }
    }
  }

  return true;
}

DynArray<const FccMatch*>
DependencyGraph::get_valid_exec_sequence()
{
  DynArray<const FccMatch*> ret;
  std::set<DGNode*> visited_nodes;
  uint32_t size = p_nodes.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    DGNode* next_root = p_nodes[i];
    if(next_root->p_parents.size() == 0 && visited_nodes.find(next_root) == visited_nodes.end())
    {
      DynArray<DGNode*> next_frontier;
      DynArray<DGNode*> current_frontier;
      current_frontier.append(next_root);
      visited_nodes.insert(next_root);
      while(current_frontier.size() > 0)
      {
        for(uint32_t ii = 0; ii < current_frontier.size(); ++ii)
        {
          DGNode* next_root = current_frontier[ii]; 
          ret.append(next_root->p_match);
          uint32_t num_children =  next_root->p_children.size();
          for(uint32_t j = 0; j < num_children; ++j)
          {
            DGNode* child = next_root->p_children[j];
            if(visited_nodes.find(child) == visited_nodes.end())
            {
              next_frontier.append(child);
              visited_nodes.insert(child);
            }
          }
        }
        current_frontier = next_frontier;
        next_frontier.clear();
      }
    }
  }
  return ret;
}

} /* furious
 */ 
