

#ifndef _FURIOUS_H_
#define _FURIOUS_H_

#include "common/common.h"
#include "runtime/runtime.h"
#include "lang/macros.h"
#include <stddef.h>
#include <string.h>

namespace furious
{

extern void 
furious_init(database_t* database);

extern void 
furious_frame(float delta, 
              database_t* database,
              void* user_data);

extern void 
furious_post_frame(float delta, 
                   database_t* database,
                   void* user_data);

extern task_graph_t* 
furious_task_graph();

extern task_graph_t* 
furious_post_task_graph();

extern void 
furious_release();

void
furious_set_log_func(log_func_t log_function);
}
#endif /* ifndef _FURIOUS_H_ */
