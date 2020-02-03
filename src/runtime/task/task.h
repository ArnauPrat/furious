


#ifndef _FURIOUS_TASK_H_
#define _FURIOUS_TASK_H_

#include "../../common/platform.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_database_t fdb_database_t;
typedef struct fdb_barrier_t
{
  void* p_state;
  void (*p_wait) (void* state, int32_t num_processes);
  void (*p_reset) ();
} fdb_barrier_t;

typedef void (*fdb_task_func_t) (float, // delta 
                              fdb_database_t*, // database
                              void*, // userdata
                              uint32_t, // chunksize 
                              uint32_t,  // offset
                              uint32_t, // stride
                              fdb_barrier_t* barrier); 

typedef struct fdb_task_t 
{
  uint32_t           m_id;
  const char*        p_info;
  fdb_task_func_t        p_func;
  bool               m_requires_sync;
  uint32_t           m_parents[FCC_MAX_TASK_PARENTS];
  uint32_t           m_children[FCC_MAX_TASK_CHILDREN];
  uint32_t           m_num_parents;
  uint32_t           m_num_children;
} fdb_task_t;

typedef struct fdb_task_graph_t
{
  fdb_task_t*           m_tasks;
  uint32_t*             m_roots;
  uint32_t              m_num_tasks;
  uint32_t              m_num_roots;
} fdb_task_graph_t;

void
fdb_task_graph_init(fdb_task_graph_t* task_graph, 
                    uint32_t num_tasks,
                    uint32_t num_roots);

void
fdb_task_graph_release(fdb_task_graph_t* task_graph);


void
fdb_task_graph_insert_task(fdb_task_graph_t* task_graph,
                       uint32_t task_id,
                       fdb_task_func_t func,
                       bool requires_sync,
                       const char* info);

void
fdb_task_graph_set_parent(fdb_task_graph_t* task_graph,
                      uint32_t task_id,
                      uint32_t parent_id);

void
fdb_task_graph_set_root(fdb_task_graph_t* task_graph,
                    uint32_t root_idx,
                    uint32_t task_id);

void
fdb_task_graph_run(fdb_task_graph_t* task_graph,
               float delta,
               fdb_database_t* database,
               void* user_data);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_TASK_H_ */
