


#ifndef _FURIOUS_TASK_H_
#define _FURIOUS_TASK_H_ value

#include "../../common/types.h"
#include "../../common/dyn_array.h"

namespace furious
{

class Database;
class barrier_t;

using task_func_t = void (*) (float, // delta 
                              Database*, // database
                              void*, // userdata
                              uint32_t, // chunksize 
                              uint32_t,  // offset
                              uint32_t, // stride
                              barrier_t* barrier); 

struct task_t 
{
  uint32_t           m_id;
  const char*        p_info;
  task_func_t        p_func;
  bool               m_requires_sync;
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
                       task_func_t func,
                       bool requires_sync,
                       const char* info);

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


class barrier_t
{
public:

  /**
   * \brief blocks the current thread/task until the counter is zero
   */
  virtual void
  wait(int32_t num_processes) = 0;

  virtual void
  reset() = 0;

};
  
}  

#endif /* ifndef _FURIOUS_TASK_H_ */
