
#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include "../common/dyn_array.h"
#include <string>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/Type.h>

using namespace clang;

namespace clang
{
class ASTContext;
class Decl;
}

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
get_code(const SourceManager &sm,
         SourceLocation &start,
         SourceLocation &end);


struct Dependency 
{
  std::string m_include_file = "";
  const Decl*       p_decl         = nullptr;
};

/**
 * @brief Gets the dependencies of a given declaration
 *
 * @param decl The declaration to get the dependencies from
 *
 * @return Returns a vector with the dependencies of a given declaration
 */
DynArray<Dependency> 
get_dependencies(const Decl* decl);

DynArray<Dependency> 
get_dependencies(const QualType& type);

std::string 
get_type_name(QualType type);

std::string 
get_qualified_type_name(QualType type);

const Decl*
get_type_decl(const QualType& type);
  
} /* furious
 */ 

#endif

