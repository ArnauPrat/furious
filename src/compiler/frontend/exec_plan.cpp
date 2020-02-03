
#include "frontend/transforms.h"
#include "exec_plan.h"
#include "fcc_context.h"
#include "../drivers/clang/clang_tools.h"
#include "../driver.h"
#include "operator.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static bool
is_dependent(const fcc_exec_plan_t* exec_plan, 
             uint32_t node_a, 
             uint32_t node_b)
{

  const fcc_stmt_t* match_b = exec_plan->p_stmts[node_b];

  fcc_component_match_t* component_matches_b = match_b->p_system->m_cmatches;
  uint32_t size_b = match_b->p_system->m_ncmatches;
  fcc_type_t write_types_b[size_b];
  uint32_t nwrite_b = 0;
  for(uint32_t i = 0; i < size_b; ++i)
  {
    fcc_type_t type = component_matches_b[i].m_type;
    bool is_read_only = component_matches_b[i].m_is_read_only;
    if(!is_read_only)
    {
      write_types_b[nwrite_b++] = type;
    }
  }

  const fcc_stmt_t* match_a = exec_plan->p_stmts[node_a];
  fcc_component_match_t* component_matches_a = match_a->p_system->m_cmatches;
  uint32_t size_a = match_a->p_system->m_ncmatches;
  uint32_t priority_a = match_a->m_priority;
  for(uint32_t i = 0; i < size_a; ++i)
  {
    fcc_type_t type_a = component_matches_a[i].m_type;
    char name_a[FCC_MAX_TYPE_NAME];
    fcc_type_name(type_a,
                  name_a,
                  FCC_MAX_TYPE_NAME);

    bool is_read_only = component_matches_a[i].m_is_read_only;

    size_b = nwrite_b; 
    uint32_t priority_b = match_b->m_priority;
    for(uint32_t ii = 0; ii < size_b; ++ii)
    {
      fcc_type_t type_b = write_types_b[ii];
      char name_b[FCC_MAX_TYPE_NAME];
      fcc_type_name(type_b,
                    name_b,
                    FCC_MAX_TYPE_NAME);
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
fcc_exec_plan_insert(fcc_exec_plan_t* exec_plan, 
                     const fcc_stmt_t* stmt)
{
  FDB_PERMA_ASSERT(exec_plan->m_nnodes < exec_plan->m_maxnodes && "Number of nodes in execution plan exceeded");

  uint32_t id = exec_plan->m_nnodes;
  exec_plan->m_nnodes++;
  exec_plan->p_stmts[id] = stmt;
  exec_plan->m_subplans[id] = new fcc_subplan_t;

  // Adding for dependencies (if any)
  uint32_t num_nodes = exec_plan->m_nnodes;
  fcc_exec_plan_node_t* added_node = &exec_plan->m_nodes[id];
  const fcc_stmt_t* added_node_match = exec_plan->p_stmts[id];
  char added_system_name[FCC_MAX_TYPE_NAME];
  fcc_type_name(added_node_match->p_system->m_system_type,
                added_system_name,
                FCC_MAX_TYPE_NAME);

  subplan_init(added_node_match, 
               exec_plan->m_subplans[id]);
  exec_plan->m_subplans[id]->m_id = id;

  for(uint32_t i = 0; 
      i < num_nodes - 1; 
      ++i)
  {
    fcc_exec_plan_node_t* next_node = &exec_plan->m_nodes[i];
    const fcc_stmt_t* next_node_match = exec_plan->p_stmts[i];
    char next_system_name[FCC_MAX_TYPE_NAME];
    fcc_type_name(next_node_match->p_system->m_system_type,
                  next_system_name,
                  FCC_MAX_TYPE_NAME);


    if(is_dependent(exec_plan, id, i))
    {
      uint32_array_append(&next_node->m_children, &id);
      uint32_array_append(&added_node->m_parents, &i);
      printf("%s depends on %s \n", 
             added_system_name,
             next_system_name);
    }

    if(is_dependent(exec_plan, i, id))
    {
      uint32_array_append(&added_node->m_children, &i);
      uint32_array_append(&next_node->m_parents, &id);
      printf("%s depends on %s \n", 
             next_system_name,
             added_system_name);
    }
  }

  // Re-evaluating roots
  exec_plan->m_nroots = 0;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(exec_plan->m_nodes[i].m_parents.m_count == 0)
    {
      FDB_PERMA_ASSERT(exec_plan->m_nroots < exec_plan->m_maxnodes && "Number of nodes in execution plan exceeded");
      exec_plan->m_roots[exec_plan->m_nroots++] = i;
    }
  }
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
  const uint32_t size = node->m_children.m_count;
  for(uint32_t i = 0; i < size; ++i)
  {
    const uint32_t child_id = node->m_children.m_data[i];
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
  uint32_t num_nodes = exec_plan->m_nnodes;
  bool visited[num_nodes];
  memset(visited, 0, sizeof(bool)*num_nodes);
  uint32_t departure[num_nodes];
  memset(departure, 0, sizeof(uint32_t)*num_nodes);

  uint32_t time=0;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(!visited[i] && exec_plan->m_nodes[i].m_parents.m_count == 0)
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
    const uint32_t num_children = node->m_children.m_count;
    for(uint32_t ii = 0; ii < num_children; ++ii)
    {
      const uint32_t child_id = node->m_children.m_data[ii];
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
  const uint32_t* parents = exec_plan->m_nodes[node_id].m_parents.m_data;
  const uint32_t num_elements = exec_plan->m_nodes[node_id].m_parents.m_count;
  for(uint32_t i = 0; i < num_elements; ++i)
  {
    if(!visited[parents[i]])
    {
      return false;
    }
  }
  return true;
}

void
fcc_exec_plan_get_valid_exec_sequence(const fcc_exec_plan_t* exec_plan, 
                                      uint32_t* seq, 
                                      uint32_t nseq)
{
  FDB_ASSERT(nseq <= exec_plan->m_maxnodes && "Sequence buffer too small");
  const uint32_t num_nodes = exec_plan->m_nnodes;
  bool visited_nodes[num_nodes];
  memset(visited_nodes, 0, sizeof(bool)*num_nodes);
  bool found = true;
  uint32_t next = 0;
  while(found)
  {
    found = false;
    for (uint32_t i = 0; i < num_nodes; ++i) 
    {
      if(!visited_nodes[i] && all_visited_parents(exec_plan, i, visited_nodes))
      {
        found = true;
        uint32_t next_frontier[num_nodes];
        uint32_t nfsize = 0;
        uint32_t current_frontier[num_nodes];
        uint32_t cfsize = 0;
        current_frontier[cfsize++] = i;
        visited_nodes[i] = true;
        while(cfsize > 0)
        {
          for(uint32_t ii = 0; ii < cfsize; ++ii)
          {
            uint32_t next_node_id = current_frontier[ii];
            seq[next++] = next_node_id;
            const fcc_exec_plan_node_t* next_node = &exec_plan->m_nodes[next_node_id]; 
            const uint32_t* children = next_node->m_children.m_data;
            const uint32_t nchildren = next_node->m_children.m_count;
            for(uint32_t j = 0; j < nchildren; ++j)
            {
              const uint32_t child_id = children[j];
              if(!visited_nodes[child_id] && all_visited_parents(exec_plan, child_id, visited_nodes))
              {
                next_frontier[nfsize++] = child_id;
                visited_nodes[child_id] = true;
              }
            }
          }
          memcpy(current_frontier, next_frontier, sizeof(uint32_t)*num_nodes);
          cfsize = nfsize;
          nfsize = 0;
        }
      }
    }
  }
}

fcc_compilation_error_type_t
fcc_exec_plan_init(fcc_exec_plan_t* exec_plan, 
                     const fcc_stmt_t* stmts[], 
                     uint32_t num_stmts)
{
  memset(exec_plan, 0, sizeof(fcc_exec_plan_t));
  exec_plan->m_nnodes = 0;
  exec_plan->m_nroots = 0;
  exec_plan->m_maxnodes = num_stmts;
  exec_plan->m_nodes = new fcc_exec_plan_node_t[exec_plan->m_maxnodes];
  exec_plan->m_roots = new uint32_t[exec_plan->m_maxnodes];
  exec_plan->p_stmts = new const fcc_stmt_t*[exec_plan->m_maxnodes];
  exec_plan->m_subplans = new fcc_subplan_t*[exec_plan->m_maxnodes];

  memset(exec_plan->m_nodes, 0, sizeof(fcc_exec_plan_node_t*)*exec_plan->m_maxnodes);
  memset(exec_plan->m_roots, 0, sizeof(uint32_t)*exec_plan->m_maxnodes);
  memset(exec_plan->p_stmts, 0, sizeof(fcc_stmt_t*)*exec_plan->m_maxnodes);
  memset(exec_plan->m_subplans, 0, sizeof(fcc_subplan_t*)*exec_plan->m_maxnodes);

  for(uint32_t i = 0; i < exec_plan->m_maxnodes; ++i)
  {
    exec_plan->m_nodes[i].m_parents = uint32_array_init();
    exec_plan->m_nodes[i].m_children = uint32_array_init();
  }

  for(uint32_t i = 0; i < exec_plan->m_maxnodes; ++i)
  {
      fcc_exec_plan_insert(exec_plan, stmts[i]);
  }

  if(!is_acyclic(exec_plan)) 
  {
    return fcc_compilation_error_type_t::E_CYCLIC_DEPENDENCY_GRAPH;
  }

  return fcc_compilation_error_type_t::E_NO_ERROR;
}

void
fcc_exec_plan_release(fcc_exec_plan_t* exec_plan)
{
  uint32_t num_nodes = exec_plan->m_nnodes;
  for(uint32_t i = 0; 
      i < num_nodes; 
      ++i)
  {
    subplan_release(exec_plan->m_subplans[i]);
    delete exec_plan->m_subplans[i];
  }

  for(uint32_t i = 0; i < exec_plan->m_nnodes; ++i)
  {
    uint32_array_release(&exec_plan->m_nodes[i].m_parents);
    uint32_array_release(&exec_plan->m_nodes[i].m_children);
  }

  delete [] exec_plan->m_nodes;
  delete [] exec_plan->m_roots;
  delete [] exec_plan->p_stmts;
  delete [] exec_plan->m_subplans;
}
