

#include "task.h"
#include "../../common/memory/memory.h"
#include "../../common/memory/numa_alloc.h"
#include <string.h>

void
fdb_task_graph_init(struct fdb_task_graph_t* task_graph, 
                    uint32_t num_tasks,
                    uint32_t num_roots)
{
  task_graph->m_tasks = (struct fdb_task_t*)fdb_numa_alloc(NULL, 
                                                   64, 
                                                   sizeof(struct fdb_task_t)*num_tasks, 
                                                   FDB_NO_HINT);

  task_graph->m_roots = (uint32_t*)fdb_numa_alloc(NULL, 
                                                   64, 
                                                   sizeof(uint32_t)*num_roots, 
                                                   FDB_NO_HINT);
  task_graph->m_num_tasks = num_tasks;
  task_graph->m_num_roots = num_roots;
}


void
fdb_task_graph_release(struct fdb_task_graph_t* task_graph)
{
  fdb_numa_free(NULL, task_graph->m_tasks);
  fdb_numa_free(NULL, task_graph->m_roots);
}


void
fdb_task_graph_insert_task(struct fdb_task_graph_t* task_graph,
                           uint32_t task_id,
                           fdb_task_func_t func,
                           bool requires_sync,
                           const char* info)
{
  memset(&task_graph->m_tasks[task_id], 0, sizeof(struct fdb_task_t));
  task_graph->m_tasks[task_id].m_id = task_id;
  task_graph->m_tasks[task_id].p_info = info;
  task_graph->m_tasks[task_id].p_func = func;
  task_graph->m_tasks[task_id].m_requires_sync = requires_sync;
}

void
fdb_task_graph_set_parent(struct fdb_task_graph_t* task_graph,
                          uint32_t task_id,
                          uint32_t parent_id)
{
  struct fdb_task_t* c = &task_graph->m_tasks[task_id];
  struct fdb_task_t* p = &task_graph->m_tasks[parent_id];

  FDB_ASSERT(c->m_num_parents < FCC_MAX_TASK_PARENTS && "Task maximum number of parents exceeded");
  FDB_ASSERT(p->m_num_parents < FCC_MAX_TASK_CHILDREN && "Task maximum number of children exceeded");

  c->m_parents[c->m_num_parents++] = parent_id;
  p->m_children[p->m_num_children++] = task_id;
}

void
fdb_task_graph_set_root(struct fdb_task_graph_t* task_graph,
                        uint32_t root_idx,
                        uint32_t task_id)
{
  task_graph->m_roots[root_idx] = task_id;
}

static bool
all_visited_parents(const struct fdb_task_graph_t* task_graph, 
                    uint32_t node_id,
                    bool * visited)
{
  const uint32_t* parents = &task_graph->m_tasks[node_id].m_parents[0];
  const uint32_t nparents = task_graph->m_tasks[node_id].m_num_parents; 
  for(uint32_t i = 0; i < nparents; ++i)
  {
    if(!visited[parents[i]])
    {
      return false;
    }
  }
  return true;
}

void
fdb_task_graph_run(struct fdb_task_graph_t* task_graph,
                   float delta,
                   struct fdb_database_t* database,
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
        uint32_t nfrontier[task_graph->m_num_tasks];
        memset(nfrontier, 0, sizeof(uint32_t)*task_graph->m_num_tasks);
        uint32_t cfrontier[task_graph->m_num_tasks];
        memset(cfrontier, 0, sizeof(uint32_t)*task_graph->m_num_tasks);
        uint32_t cnfrontier = 0;
        uint32_t ccfrontier = 0;
        cfrontier[ccfrontier++] = i;
        visited_nodes[i] = true;
        while(ccfrontier > 0)
        {
          for(uint32_t ii = 0; ii < ccfrontier; ++ii)
          {
            const uint32_t next_node_id = cfrontier[ii];
            const struct fdb_task_t* next_node = &task_graph->m_tasks[next_node_id]; 
            next_node->p_func(delta, database, user_data, 1, 0, 1, NULL);
            const uint32_t* children = next_node->m_children;
            const uint32_t num_children = next_node->m_num_children;
            for(uint32_t j = 0; j < num_children; ++j)
            {
              const uint32_t child_id = children[j];
              if(!visited_nodes[child_id] && all_visited_parents(task_graph, child_id, visited_nodes))
              {
                nfrontier[cnfrontier++] = child_id;
                visited_nodes[child_id] = true;
              }
            }
          }
          memcpy(cfrontier, nfrontier, sizeof(uint32_t)*task_graph->m_num_tasks);
          ccfrontier = cnfrontier;
          memset(nfrontier, 0, sizeof(uint32_t)*task_graph->m_num_tasks);
          cnfrontier = 0;
        }
      }
    }
  }
}
