


#ifndef _FURIOUS_COMPILER_PARSING_H_
#define _FURIOUS_COMPILER_PARSING_H_ value

#include <string>
#include <clang/AST/AST.h>

using namespace clang;

namespace furious 
{

struct FccContext;
struct FccMatch;
struct FccEntityMatch;
struct FccSystem;
enum class FccParsingErrorType;

/**
 * @brief Provess a furious entry point call
 *
 * @param ast_context The ast context of the call to process
 * @param exec_info The FccExecInfo where the result must be stored
 * @param call The CallExpr representing the call
 */
bool 
process_entry_point(ASTContext* ast_context,
                    FccContext* fcc_context,
                    FccSystem*  fcc_system,
                    const CallExpr* call);

/**
 * @brief Process a with_tag call
 *
 * @param ast_context The ast context of the with_tag call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool 
process_has_tag(ASTContext* ast_context,
                 FccContext *fcc_context,
                 FccEntityMatch* entity_match,
                 const CallExpr* call);

/**
 * @brief Process a has_not_tag call
 *
 * @param ast_context The ast context of the has_not_tag call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool 
process_has_not_tag(ASTContext* ast_context,
                    FccContext *fcc_context,
                    FccEntityMatch* entity_match,
                    const CallExpr* call);

/**
 * @brief Process a with_component call
 *
 * @param ast_context The ast context of the with_component call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool 
process_has_component(ASTContext* ast_context,
                       FccContext *fcc_context,
                       FccEntityMatch* entity_match,
                       const CallExpr* call);

/**
 * @brief Process a without_component call
 *
 * @param ast_context The ast context of the has_not_component call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool 
process_has_not_component(ASTContext* ast_context,
                          FccContext *fcc_context,
                          FccEntityMatch* entity_match,
                          const CallExpr* call);

/**
 * @brief Process a filter call
 *
 * @param ast_context The ast context of the filter call
 * @param exec_info The exec info where the result is going to be stored
 * @param call The AST call expression to process
 */
bool 
process_filter(ASTContext* ast_context,
               FccContext *fcc_context,
               FccEntityMatch* exec_info,
               const CallExpr* call);



void 
report_parsing_error(ASTContext* ast_context, 
                     FccContext* fcc_context,
                     const SourceLocation& location,
                     const FccParsingErrorType& type);

} /* furious */ 

#endif /* ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_ */
