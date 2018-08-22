


#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include <string>
#include <clang/AST/AST.h>
#include <clang/Lex/Lexer.h>

using namespace clang;

namespace furious 
{

struct FccExecInfo;

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
std::string get_code(SourceManager &sm,
                     SourceLocation &start,
                     SourceLocation &end);

/**
 * @brief Given a TemplateArgumentList, extract the QualTypes of the template
 * argument types
 *
 * @param arg_list The TemplateArgumentList to extract the types from
 *
 * @return Returns a std::vector with the extracted QualTypes
 */
std::vector<QualType> 
get_tmplt_types(const TemplateArgumentList& arg_list);

/**
 * @brief Extract the execution information from 
 *
 * @param decl
 *
 * @return 
 */
void extract_basic_info(FccExecInfo* exec_info, 
                         const FunctionDecl* func_decl);

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_ */
