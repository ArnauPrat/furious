


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
 * @brief Provess a furious entry point call
 *
 * @param ast_context The ast context of the call to process
 * @param exec_info The FccExecInfo where the result must be stored
 * @param call The CallExpr representing the call
 */
bool process_entry_point(ASTContext* ast_context,
                         FccContext *fcc_context,
                         FccExecInfo* exec_info,
                         const CallExpr* call);

/**
 * @brief Process a with_tag call
 *
 * @param ast_context The ast context of the with_tag call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool process_with_tag(ASTContext* ast_context,
                      FccContext *fcc_context,
                      FccExecInfo* exec_info,
                      const CallExpr* call);

/**
 * @brief Process a without_tag call
 *
 * @param ast_context The ast context of the without_tag call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool process_without_tag(ASTContext* ast_context,
                         FccContext *fcc_context,
                         FccExecInfo* exec_info,
                         const CallExpr* call);

/**
 * @brief Process a with_component call
 *
 * @param ast_context The ast context of the with_component call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool process_with_component(ASTContext* ast_context,
                            FccContext *fcc_context,
                            FccExecInfo* exec_info,
                            const CallExpr* call);

/**
 * @brief Process a without_component call
 *
 * @param ast_context The ast context of the without_component call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool process_without_component(ASTContext* ast_context,
                               FccContext *fcc_context,
                               FccExecInfo* exec_info,
                               const CallExpr* call);

/**
 * @brief Process a filter call
 *
 * @param ast_context The ast context of the filter call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool process_filter(ASTContext* ast_context,
                    FccContext *fcc_context,
                    FccExecInfo* exec_info,
                    const CallExpr* call);



void report_parsing_error(ASTContext* ast_context, 
                          FccContext* fcc_context,
                          const SourceLocation& location,
                          const FccParsingErrorType& type);

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_ */
