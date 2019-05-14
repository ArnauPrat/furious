

#ifndef _FURIOUS_FCC_REFLECTION_H_
#define _FURIOUS_FCC_REFLECTION_H_ value

#include <stdio.h>

namespace clang
{
class CXXRecordDecl;
}

using namespace clang;

namespace furious
{

void
generate_reflection_code(FILE* fp, CXXRecordDecl* decl);
  
} /*  furious */ 

#endif /* ifndef _FURIOUS_FCC_REFLECTION_H_ */
