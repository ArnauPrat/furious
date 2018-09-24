
#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

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
get_code(SourceManager &sm,
         SourceLocation &start,
         SourceLocation &end);


struct Dependency 
{
  std::string m_include_file = "";
  Decl*       p_decl         = nullptr;
};

/**
 * @brief Gets the dependencies of a given declaration
 *
 * @param decl The declaration to get the dependencies from
 *
 * @return Returns a vector with the dependencies of a given declaration
 */
std::vector<Dependency> 
get_dependencies(ASTContext* context, 
                 const Decl* decl);

std::string 
get_type_name(const QualType& type);
  
} /* furious
 */ 

#endif

