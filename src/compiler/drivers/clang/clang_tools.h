
#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include "../common/dyn_array.h"
#include "fcc_context.h"
#include "../../backend/codegen_tools.h"

#include <string>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/Type.h>

#include "../../driver.h"

using namespace clang;

namespace clang
{
class ASTContext;
class Decl;
}

namespace furious
{

/**
 * \brief Gets the access mode of the component represented by the given
 * QualType
 *
 * \param type The QualType representing the component
 *
 * \return The access mode of the component
 */
fcc_access_mode_t
get_access_mode(const QualType& type);

/**
 * \brief Gets the declaration of a QualType
 *
 * \param type The QualType to get the declaration from
 *
 * \return A pointer to the Declaration of the given QualType
 */
Decl*
get_type_decl(const QualType& type);

/**
 * \brief Gets the line number of a source location
 *
 * \param manager The source manager the source location depends on
 * \param location The source location
 *
 * \return Returns the line represented by the given source location
 */
uint32_t 
get_line_number(const SourceManager& manager,
                const SourceLocation& location);

/**
 * \brief Gets the column number of a source location
 *
 * \param manager The source manager the source location depends on
 * \param location The source location
 *
 * \return Returns the column represented by the given source location
 */
uint32_t 
get_column_number(const SourceManager& manager,
                  const SourceLocation& location);


/**
 * \brief Gets the filename of a source location
 *
 * \param manager The source manager
 * \param location The location
 * \param buffer The buffer to store the filename
 * \param buffer_lenght The length of the buffer
 *
 * \return  The length of the filname
 */
uint32_t
get_filename(const SourceManager& manager,
             const SourceLocation& location,
             char* buffer,
             uint32_t buffer_length);


/**
 * \brief Gets the string literal of an expression
 *
 * \param expr The expression to get the literal of
 * \param buffer  The buffer to store the literal to
 * \param buffer_length The length of the buffer
 *
 * \return Returns the lenght of the literal
 */
uint32_t 
get_string_literal(const Expr* expr,
                   char * buffer,
                   uint32_t buffer_length);

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
 * \brief Gets the name of a QualType
 *
 * \param type The QualType to get the name from
 * \param buffer The buffer where the name will be written to 
 * \param type The length of the buffer 
 *
 * \return The length of the type name
 */
uint32_t 
get_type_name(const QualType& type,
              char* buffer, 
              uint32_t buffer_length);

/**
 * \brief Gets the name of a QualType, including the namespace
 *
 * \param type The QualType to get the name from
 * \param buffer The buffer where the name will be written to 
 * \param type The length of the buffer 
 *
 * \return Returns the length of the tagged type name
 */
uint32_t
get_tagged_type_name(const QualType& type,
                     char* buffer, 
                     uint32_t buffer_length);

/**
 * \brief Gets the fully qualified name of a QualType
 *
 * \param type The QualType to get the fully qualified type name from
 * \param buffer The buffer where the name will be written to 
 * \param type The length of the buffer 
 *
 * \return Returns the length of the qualified type name
 */

uint32_t 
get_qualified_type_name(const QualType& type,
                        char* buffer, 
                        uint32_t buffer_length);


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

