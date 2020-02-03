
#ifndef _FURIOUS_FCCASTVISITOR_H_
#define _FURIOUS_FCCASTVISITOR_H_

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "fcc_context.h"

using namespace clang;


struct FccContext;

/**
* @brief Visitor that extracts the execution information of a an expression with
* cleanups.
*/
class FuriousExprVisitor : public RecursiveASTVisitor<FuriousExprVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info

public:
  fcc_stmt_t* p_fcc_stmt;

  FuriousExprVisitor(ASTContext *ast_context);

  fcc_stmt_t*  
  parse_expression(Expr* expr);

  virtual 
  bool TraverseLambdaBody(LambdaExpr* expr);

  virtual 
  bool VisitCallExpr(CallExpr* call);

};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * @brief Visitor that triggers a FuriousScriptVisitor on the body of the
 * furious_script function.
 */
class FccASTVisitor : public RecursiveASTVisitor<FccASTVisitor>
{
private:
  ASTContext* p_ast_context; // used for getting additional AST info

public:
  explicit FccASTVisitor(ASTContext* ast_context);

  virtual 
  bool VisitFunctionDecl(FunctionDecl* func);
};

#endif /* ifndef _FURIOUS_CLANGASTVISITOR_H_ */
