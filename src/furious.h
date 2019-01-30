

#ifndef _FURIOUS_H_
#define _FURIOUS_H_

#include "common/common.h"
#include "runtime/runtime.h"

namespace furious
{
extern void __furious_init(Database* database);
extern void __furious_frame(float delta, 
                            Database* database);
extern void __furious_release();

}
#endif /* ifndef _FURIOUS_H_ */
