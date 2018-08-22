
#ifndef _FURIOUS_FCCASTVISITOR_H_
#define _FURIOUS_FCCASTVISITOR_H_

#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>

#include "structs.h"

using namespace clang;

namespace furious
{

/**
* @brief Visitor that extracts the execution information of a an expression with
* cleanups.
*/
class FuriousExprVisitor : public RecursiveASTVisitor<FuriousExprVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info
  FccContext *p_fcc_context;
  FccExecInfo m_fcc_exec_info;

public:
  explicit FuriousExprVisitor(ASTContext *ast_context,
                              FccContext *fcc_context);

  virtual 
  bool VisitExprWithCleanups(ExprWithCleanups *expr);

  virtual
  bool VisitCXXMemberCallExpr(CXXMemberCallExpr *expr);

  virtual 
  bool VisitCallExpr(CallExpr *func);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * @brief Visitor triggered on the body of the furious script (code within
 * BEGIN_FURIOUS and END_FURIOUS). It is responsible of triggering a
 * FunctionExprVisitor on every ExpressionWithCleanUps found within such macros
 */
class FuriousScriptVisitor : public RecursiveASTVisitor<FuriousScriptVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info
  FccContext *p_fcc_context;

public:
  explicit FuriousScriptVisitor(CompilerInstance* cI,
                                FccContext *fcc_context);

  virtual 
  bool VisitCXXRecordDecl(CXXRecordDecl *record);

  virtual 
  bool VisitLambdaExpr(LambdaExpr *lamnda);

  virtual 
  bool VisitExprWithCleanups(ExprWithCleanups *expr);
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
  ASTContext *p_ast_context; // used for getting additional AST info
  FuriousScriptVisitor *p_script_visitor;
  FccContext *p_fcc_context;

public:
  explicit FccASTVisitor(CompilerInstance *CI,
                         FccContext *fcc_context);

  virtual 
  bool VisitFunctionDecl(FunctionDecl *func);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class FccASTConsumer : public ASTConsumer
{
private:
  FccASTVisitor *visitor; // doesn't have to be private

public:
  // override the constructor in order to pass CI
  explicit FccASTConsumer(CompilerInstance *cI,
                          FccContext *fcc_context);

  // override this to call our ExampleVisitor on the entire source file
  virtual void
  HandleTranslationUnit(ASTContext &context);
};

class FccFrontendAction : public ASTFrontendAction
{
public:
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &cI, StringRef file);
};

} // namespace furious

#endif /* ifndef _FURIOUS_CLANGASTVISITOR_H_ */
