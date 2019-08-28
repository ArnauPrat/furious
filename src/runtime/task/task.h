


#ifndef _FURIOUS_TASK_H_
#define _FURIOUS_TASK_H_ value

#include "../../common/types.h"
#include "../../common/dyn_array.h"

namespace furious
{

class Database;

using task_func_t = void (*) (float, // delta 
                              Database*, // database
                              void*, // userdata
                              uint32_t, // chunksize 
                              uint32_t,  // offset
                              uint32_t); // stride

struct task_t 
{
  task_func_t        p_func;
  DynArray<uint32_t> m_parents;
  DynArray<uint32_t> m_children;
};

struct task_graph_t
{
  task_t*               m_tasks;
  uint32_t*             m_roots;
  uint32_t              m_num_tasks;
  uint32_t              m_num_roots;
};

task_graph_t* 
task_graph_create(uint32_t num_tasks,
                  uint32_t num_roots);

void
task_graph_destroy(task_graph_t* task_graph);


void
task_graph_insert_task(task_graph_t* task_graph,
            uint32_t task_id,
            task_func_t func);

void
task_graph_set_parent(task_graph_t* task_graph,
                uint32_t task_id,
                uint32_t parent_id);

void
task_graph_set_root(task_graph_t* task_graph,
                    uint32_t root_idx,
                    uint32_t task_id);

void
task_graph_run(task_graph_t* task_graph,
               float delta,
               Database* database,
               void* user_data);
  
}  

#endif /* ifndef _FURIOUS_TASK_H_ */
