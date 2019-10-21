
#include "frontend/transforms.h"
#include "exec_plan.h"
#include "fcc_context.h"
#include "../drivers/clang/clang_tools.h"
#include "../driver.h"
#include "operator.h"

namespace furious 
{


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static bool
is_dependent(const fcc_exec_plan_t* exec_plan, 
             uint32_t node_a, 
             uint32_t node_b)
{
  DynArray<fcc_type_t> write_types_b;

  const fcc_stmt_t* match_b = exec_plan->p_stmts[node_b];
  const DynArray<fcc_component_match_t>& component_matches_b = match_b->p_system->m_component_types;
  uint32_t size_b = component_matches_b.size(); 
  for(uint32_t i = 0; i < size_b; ++i)
  {
    fcc_type_t type = component_matches_b[i].m_type;
    bool is_read_only = component_matches_b[i].m_is_read_only;
    if(!is_read_only)
    {
      write_types_b.append(type);
    }
  }

  const fcc_stmt_t* match_a = exec_plan->p_stmts[node_a];
  const DynArray<fcc_component_match_t>& component_matches_a = match_a->p_system->m_component_types;
  uint32_t size_a = component_matches_a.size();
  uint32_t priority_a = match_a->m_priority;
  for(uint32_t i = 0; i < size_a; ++i)
  {
    fcc_type_t type_a = component_matches_a[i].m_type;
    char name_a[MAX_TYPE_NAME];
    fcc_type_name(type_a,
                  name_a,
                  MAX_TYPE_NAME);

    bool is_read_only = component_matches_a[i].m_is_read_only;

    size_b = write_types_b.size();
    uint32_t priority_b = match_b->m_priority;
    for(uint32_t ii = 0; ii < size_b; ++ii)
    {
      fcc_type_t type_b = write_types_b[ii];
      char name_b[MAX_TYPE_NAME];
      fcc_type_name(type_b,
                    name_b,
                    MAX_TYPE_NAME);
      if((strcmp(name_a, name_b) == 0) && is_read_only)
      {
        return true;
      }

      if((strcmp(name_a, name_b) == 0) && 
         priority_a >= priority_b)
      {
        return true;
      }
    }
  }
  return false;
}

void
insert(fcc_exec_plan_t* exec_plan, 
       const fcc_stmt_t* stmt)
{
  exec_plan->m_nodes.append(fcc_exec_plan_node_t());
  exec_plan->p_stmts.append(stmt);
  exec_plan->m_subplans.append(new fcc_subplan_t);

  // Adding for dependencies (if any)
  uint32_t num_nodes = exec_plan->m_nodes.size();
  uint32_t added_node_id = num_nodes -1; 
  fcc_exec_plan_node_t* added_node = &exec_plan->m_nodes[added_node_id];
  const fcc_stmt_t* added_node_match = exec_plan->p_stmts[added_node_id];
  char added_system_name[MAX_TYPE_NAME];
  fcc_type_name(added_node_match->p_system->m_system_type,
                added_system_name,
                MAX_TYPE_NAME);

  subplan_init(added_node_match, 
               exec_plan->m_subplans[added_node_id]);

  for(uint32_t i = 0; 
      i < num_nodes - 1; 
      ++i)
  {
    uint32_t next_node_id = i;
    fcc_exec_plan_node_t* next_node = &exec_plan->m_nodes[next_node_id];
    const fcc_stmt_t* next_node_match = exec_plan->p_stmts[next_node_id];
    char next_system_name[MAX_TYPE_NAME];
    fcc_type_name(next_node_match->p_system->m_system_type,
                  next_system_name,
                  MAX_TYPE_NAME);


    if(is_dependent(exec_plan, added_node_id, next_node_id))
    {
      next_node->m_children.append(added_node_id);
      added_node->m_parents.append(next_node_id);
      printf("%s depends on %s \n", 
             added_system_name,
             next_system_name);
    }

    if(is_dependent(exec_plan, next_node_id, added_node_id))
    {
      added_node->m_children.append(next_node_id);
      next_node->m_parents.append(added_node_id);
      printf("%s depends on %s \n", 
             next_system_name,
             added_system_name);
    }
  }

  // Re-evaluating roots
  exec_plan->m_roots.clear();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(exec_plan->m_nodes[i].m_parents.size() == 0)
    {
      exec_plan->m_roots.append(i);
    }
  }

  assert(exec_plan->m_nodes.size() == exec_plan->p_stmts.size() &&
         exec_plan->m_nodes.size() == exec_plan->m_subplans.size() &&
         "Inconsistent number of node attributes in ExecutionPlan");
}

void
dfs(const fcc_exec_plan_t* exec_plan, 
    bool* visited,
    uint32_t* departure,
    uint32_t* time,
    uint32_t node_id)
{
  visited[node_id] = true;
  const fcc_exec_plan_node_t* node = &exec_plan->m_nodes[node_id];
  const uint32_t size = node->m_children.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const uint32_t child_id = node->m_children[i];
    if(!visited[child_id])
    {
      dfs(exec_plan,
          visited,
          departure,
          time,
          child_id);

    }
  }

  departure[node_id] = *time;
  ++(*time);
}

bool
is_acyclic(const fcc_exec_plan_t* exec_plan)
{
  uint32_t num_nodes = exec_plan->m_nodes.size();
  bool visited[num_nodes];
  memset(visited, 0, sizeof(bool)*num_nodes);
  uint32_t departure[num_nodes];
  memset(departure, 0, sizeof(bool)*num_nodes);

  uint32_t time=0;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(!visited[i] && exec_plan->m_nodes[i].m_parents.size() == 0)
    {
      dfs(exec_plan, 
          visited, 
          departure, 
          &time, 
          i);
    }
  }

  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    uint32_t node_id = i;
    const fcc_exec_plan_node_t* node = &exec_plan->m_nodes[node_id];
    const uint32_t num_children = node->m_children.size();
    for(uint32_t ii = 0; ii < num_children; ++ii)
    {
      const uint32_t child_id = node->m_children[ii];
      if(departure[node_id] <= departure[child_id])
      {
        return false;
      }
    }
  }
  return true;
}


static bool
all_visited_parents(const fcc_exec_plan_t* exec_plan, 
                    uint32_t node_id,
                    bool * visited)
{
  const DynArray<uint32_t> parents = exec_plan->m_nodes[node_id].m_parents;
  const uint32_t num_elements = parents.size();
  for(uint32_t i = 0; i < num_elements; ++i)
  {
    if(!visited[parents[i]])
    {
      return false;
    }
  }
  return true;
}

DynArray<uint32_t>
get_valid_exec_sequence(const fcc_exec_plan_t* exec_plan)
{
  DynArray<uint32_t> ret;
  const uint32_t num_nodes = exec_plan->m_nodes.size();
  bool visited_nodes[num_nodes];
  memset(visited_nodes, 0, sizeof(bool)*num_nodes);
  bool found = true;
  while(found)
  {
    found = false;
    for (uint32_t i = 0; i < num_nodes; ++i) 
    {
      if(!visited_nodes[i] && all_visited_parents(exec_plan, i, visited_nodes))
      {
        found = true;
        DynArray<uint32_t> next_frontier;
        DynArray<uint32_t> current_frontier;
        current_frontier.append(i);
        visited_nodes[i] = true;
        while(current_frontier.size() > 0)
        {
          const uint32_t frontier_size = current_frontier.size();
          for(uint32_t ii = 0; ii < frontier_size; ++ii)
          {
            const uint32_t next_node_id = current_frontier[ii];
            ret.append(next_node_id);
            const fcc_exec_plan_node_t* next_node = &exec_plan->m_nodes[next_node_id]; 
            const DynArray<uint32_t>& children = next_node->m_children;
            const uint32_t num_children = children.size();
            for(uint32_t j = 0; j < num_children; ++j)
            {
              const uint32_t child_id = children[j];
              if(!visited_nodes[child_id] && all_visited_parents(exec_plan, child_id, visited_nodes))
              {
                next_frontier.append(child_id);
                visited_nodes[child_id] = true;
              }
            }
          }
          current_frontier = next_frontier;
          next_frontier.clear();
        }
      }
    }
  }
  return ret;
}

fcc_compilation_error_type_t
create_execplan(const fcc_stmt_t* stmts[], 
                   uint32_t num_stmts,
                   fcc_exec_plan_t** exec_plan)
{
  *exec_plan = nullptr;
  fcc_exec_plan_t* ret_exec_plan = new fcc_exec_plan_t();
  for(uint32_t i = 0; i < num_stmts; ++i)
  {
      insert(ret_exec_plan, stmts[i]);
  }

  if(!is_acyclic(ret_exec_plan)) 
  {
    return fcc_compilation_error_type_t::E_CYCLIC_DEPENDENCY_GRAPH;
  }

  *exec_plan = ret_exec_plan;
  return fcc_compilation_error_type_t::E_NO_ERROR;
}

void
destroy_execplan(fcc_exec_plan_t* exec_plan)
{
  uint32_t num_nodes = exec_plan->m_subplans.size();
  for(uint32_t i = 0; 
      i < num_nodes; 
      ++i)
  {
    subplan_release(exec_plan->m_subplans[i]);
    delete exec_plan->m_subplans[i];
  }
  delete exec_plan;
}

} /* furious */ 
