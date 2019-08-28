

#ifndef _FURIOUS_H_
#define _FURIOUS_H_

#include "common/common.h"
#include "runtime/runtime.h"
#include "lang/macros.h"
#include "stddef.h"

namespace furious
{

extern void __furious_init(Database* database);

extern void __furious_frame(float delta, 
                            Database* database,
                            void* user_data);

extern void __furious_post_frame(float delta, 
                                 Database* database,
                                 void* user_data);

extern task_graph_t* __furious_task_graph();

extern task_graph_t* __furious_post_task_graph();

extern void __furious_release();

}
#endif /* ifndef _FURIOUS_H_ */
