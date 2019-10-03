


#ifndef _FURIOUS_TASK_H_
#define _FURIOUS_TASK_H_ value

#include "../../common/types.h"
#include "../../common/dyn_array.h"

namespace furious
{

class Database;
class sync_counter_t;

using task_func_t = void (*) (float, // delta 
                              Database*, // database
                              void*, // userdata
                              uint32_t, // chunksize 
                              uint32_t,  // offset
                              uint32_t,
                              sync_counter_t* sync_counter); // stride

struct task_t 
{
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
                       bool requires_sync);

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

class sync_counter_t
{
public:

  /**
   * \brief Sets the counter to the given value
   *
   * \param val
   */
  virtual void
  set(uint32_t val) = 0;

  /**
   * \brief Decrements the value of the counter
   */
  virtual void
  decrement() = 0;

  /**
   * \brief blocks the current thread/task until the counter is zero
   */
  virtual void
  join_while_zero() = 0;

  /**
   * \brief blocks the current thread/task until the counter is zero
   */
  virtual void
  join_while_non_zero() = 0;

};
  
}  

#endif /* ifndef _FURIOUS_TASK_H_ */
