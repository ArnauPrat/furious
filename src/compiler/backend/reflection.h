

#ifndef _FURIOUS_FCC_REFLECTION_H_
#define _FURIOUS_FCC_REFLECTION_H_ value

#include <stdio.h>
#include "../fcc_context.h"

namespace furious
{

void
generate_reflection_code(FILE* fp, fcc_decl_t decl);
  
} /*  furious */ 

#endif /* ifndef _FURIOUS_FCC_REFLECTION_H_ */
