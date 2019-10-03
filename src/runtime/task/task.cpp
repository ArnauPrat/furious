

#include "task.h"
#include <string.h>

namespace furious
{

task_graph_t* 
task_graph_create(uint32_t num_tasks,
                  uint32_t num_roots)
{
  task_graph_t* task_graph = new task_graph_t();
  task_graph->m_tasks = new task_t[num_tasks];
  task_graph->m_roots = new uint32_t[num_roots];
  task_graph->m_num_tasks = num_tasks;
  task_graph->m_num_roots = num_roots;
  return task_graph;
}


void
task_graph_destroy(task_graph_t* task_graph)
{
  delete [] task_graph->m_tasks;
  delete [] task_graph->m_roots;
}


void
task_graph_insert_task(task_graph_t* task_graph,
            uint32_t task_id,
            task_func_t func,
            bool requires_sync)
{
  task_graph->m_tasks[task_id].p_func = func;
  task_graph->m_tasks[task_id].m_requires_sync = requires_sync;
}

void
task_graph_set_parent(task_graph_t* task_graph,
                uint32_t task_id,
                uint32_t parent_id)
{
  task_graph->m_tasks[task_id].m_parents.append(parent_id);
  task_graph->m_tasks[parent_id].m_children.append(task_id);
}

void
task_graph_set_root(task_graph_t* task_graph,
                    uint32_t root_idx,
                    uint32_t task_id)
{
  task_graph->m_roots[root_idx] = task_id;
}

static bool
all_visited_parents(const task_graph_t* task_graph, 
                    uint32_t node_id,
                    bool * visited)
{
  const DynArray<uint32_t> parents = task_graph->m_tasks[node_id].m_parents;
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

void
task_graph_run(task_graph_t* task_graph,
               float delta,
               Database* database,
               void* user_data)
{
  const uint32_t num_nodes = task_graph->m_num_tasks;
  bool visited_nodes[num_nodes];
  memset(visited_nodes, 0, sizeof(bool)*num_nodes);
  bool found = true;
  while(found)
  {
    found = false;
    for (uint32_t i = 0; i < num_nodes; ++i) 
    {
      if(!visited_nodes[i] && all_visited_parents(task_graph, i, visited_nodes))
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
            const task_t* next_node = &task_graph->m_tasks[next_node_id]; 
            next_node->p_func(delta, database, user_data, 1, 0, 1, nullptr);
            const DynArray<uint32_t>& children = next_node->m_children;
            const uint32_t num_children = children.size();
            for(uint32_t j = 0; j < num_children; ++j)
            {
              const uint32_t child_id = children[j];
              if(!visited_nodes[child_id] && all_visited_parents(task_graph, child_id, visited_nodes))
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
}
  
} /* furious */ 
