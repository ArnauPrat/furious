

#ifndef _FURIOUS_COMPILER_PARSING_H_
#define _FURIOUS_COMPILER_PARSING_H_ value

#include <clang/AST/AST.h>

using namespace clang;


struct fcc_context_t;
struct fcc_stmt_t;
enum class fcc_parsing_error_type_t;

/**
 * \brief Process a match call
 *
 * \param ast_context The ast context of the expand call
 * \param fcc_match The fcc_match of the expand call
 * \param call The call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_match(ASTContext* ast_context,
              fcc_stmt_t* fcc_match,
              const CallExpr* call);

/**
 * \brief Process an expand call
 *
 * \param ast_context The ast context of the expand call
 * \param fcc_match The fcc_match of the expand call
 * \param call The call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_expand(ASTContext* ast_context,
               fcc_stmt_t* fcc_match,
               const CallExpr* call);


/**
 * \brief Process a furious entry point call
 *
 * \param ast_context The ast context of the call to process
 * \param fcc_system The FccExecInfo where the result must be stored
 * \param call The CallExpr representing the call
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_foreach(ASTContext* ast_context,
                fcc_stmt_t*   fcc_match,
                const CallExpr* call);

/**
 * \brief Processes a set_priority call
 *
 * \param ast_context The ast context of the call to process
 * \param fcc_match The match 
 * \param call The call expression
 *
 * \return  True if the parsing was corret. False otherwise.
 */
bool 
process_set_priority(ASTContext* ast_context,
                  fcc_stmt_t*   fcc_match,
                  const CallExpr* call);

/**
 * \brief Processes a set_post_frame call
 *
 * \param ast_context The ast context of the call to process
 * \param fcc_match The match 
 * \param call The call expression
 *
 * \return  True if the parsing was corret. False otherwise.
 */
bool 
process_set_post_frame(ASTContext* ast_context,
                       fcc_stmt_t*   fcc_match,
                       const CallExpr* call);

/**
 * \brief Process a with_tag call
 *
 * \param ast_context The ast context of the with_tag call
 * \param fcc_match The exec info where the result is going to be stored
 * \param call The AST call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_has_tag(ASTContext* ast_context,
                 fcc_stmt_t* fcc_match,
                 const CallExpr* call);

/**
 * \brief Process a has_not_tag call
 *
 * \param ast_context The ast context of the has_not_tag call
 * \param fcc_match The exec info where the result is going to be stored
 * \param call The AST call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_has_not_tag(ASTContext* ast_context,
                    fcc_stmt_t* fcc_match,
                    const CallExpr* call);

/**
 * \brief Process a with_component call
 *
 * \param ast_context The ast context of the with_component call
 * \param fcc_match The exec info where the result is going to be stored
 * \param call The AST call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_has_component(ASTContext* ast_context,
                      fcc_stmt_t* fcc_match,
                      const CallExpr* call);

/**
 * \brief Process a without_component call
 *
 * \param ast_context The ast context of the has_not_component call
 * \param fcc_match The exec info where the result is going to be stored
 * \param call The AST call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_has_not_component(ASTContext* ast_context,
                          fcc_stmt_t* fcc_match,
                          const CallExpr* call);

/**
 * \brief Process a filter call
 *
 * \param ast_context The ast context of the filter call
 * \param fcc_match The exec info where the result is going to be stored
 * \param call The AST call expression to process
 *
 * \return True of the parsing was correct. False otherwise.
 */
bool 
process_filter(ASTContext* ast_context,
               fcc_stmt_t* fcc_match,
               const CallExpr* call);


void
report_parsing_error(const SourceManager& sm,
                     const SourceLocation& location,
                     fcc_parsing_error_type_t error_type,
                     const char* message);

#endif /* ifndef _FURIOUS_COMPILER_CLANG_TOOLS_H_ */
