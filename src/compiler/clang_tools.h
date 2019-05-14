
#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include "../common/dyn_array.h"
#include "fcc_context.h"

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
 * @param range The source range of the code
 *
 * @return Returns a string with the code in the expecified source location
 * range
 */
std::string 
get_code(const SourceManager &sm,
         SourceRange range);


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
 * @return Returns a dynamic array with the dependencies of a given declaration
 */
DynArray<Dependency> 
get_dependencies(const Decl* decl);

/**
 * \brief Gets the dependencies of a QualType 
 *
 * \param type The type to get the dependencies from
 *
 * \return Returns a dynamic array with the found dependencies
 */
DynArray<Dependency> 
get_dependencies(const QualType& type);

/**
 * \brief Gets the name of a QualType
 *
 * \param type The QualType to get the name from
 *
 * \return Returns the name of the QualType
 */
std::string 
get_type_name(const QualType& type);

/**
 * \brief Gets the name of a QualType, including the namespace
 *
 * \param type The QualType to get the name from
 *
 * \return Returns the name of the QualType
 */
std::string 
get_tagged_type_name(const QualType& type);

/**
 * \brief Gets the fully qualified name of a QualType
 *
 * \param type The QualType to get the fully qualified type name from
 *
 * \return Returns the fully qualify name of the QualType
 */
std::string 
get_qualified_type_name(const QualType& type);

/**
 * \brief Gets the declaration of a QualType
 *
 * \param type The QualType to get the declaration from
 *
 * \return A pointer to the Declaration of the given QualType
 */
const Decl*
get_type_decl(const QualType& type);


/**
 * \brief Gets the access mode of the component represented by the given
 * QualType
 *
 * \param type The QualType representing the component
 *
 * \return The access mode of the component
 */
FccAccessMode
get_access_mode(const QualType& type);


/**
 * \brief Gets the string literal of an expression
 *
 * \param expr The expression to get the string literal from
 *
 * \return Returns the string literal. Empty string if string literal cannot be
 * found
 */
std::string
get_string_literal(const Expr* expr);

/**
 * \brief Gets the uint32 literal of an expression
 *
 * \param expr The expression to get the string literal from
 *
 * \return Returns the uint32 literal. 0 if string literal cannot be
 * found
 */
uint32_t
get_uint32_literal(const Expr* expr);
  
} /* furious
 */ 

#endif

