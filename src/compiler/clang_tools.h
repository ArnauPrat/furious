
#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include <string>
#include <clang/Basic/SourceManager.h>

using namespace clang;

namespace furious
{
/**
 * @brief Gets the code between start and end SourceLocations.
 *
 * @param sm The source manager to get the code from
 * @param start The start location of the code
 * @param end The end location of the code.
 *
 * @return Returns a string with the code in the expecified source location
 * range
 */
std::string 
get_code(SourceManager &sm,
         SourceLocation &start,
         SourceLocation &end);
  
} /* furious
 */ 

#endif

