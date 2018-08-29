


#ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_
#define _FURIOUS_COMPILER_CLANG_TOOLS_H_ value

#include <string>
#include <clang/AST/AST.h>
#include <clang/Lex/Lexer.h>

using namespace clang;

namespace furious 
{

struct FccContext;
struct FccExecInfo;
enum class FccParsingErrorType;

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

/**
 * @brief Extract the execution information from 
 *
 * @param decl
 *
 * @return 
 */
bool extract_exec_info(ASTContext* context,
                       FccExecInfo* exec_info, 
                       const CallExpr* call);

void report_parsing_error(ASTContext* ast_context, 
                          FccContext* fcc_context,
                          const SourceLocation& location,
                          const FccParsingErrorType& type);

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_ */
