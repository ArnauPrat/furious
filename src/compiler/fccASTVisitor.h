
#ifndef _FURIOUS_FCCASTVISITOR_H_
#define _FURIOUS_FCCASTVISITOR_H_

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Lexer.h>

#include "structs.h"

using namespace clang;

namespace furious
{

class FuriousExprVisitor : public RecursiveASTVisitor<FuriousExprVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info
  FccContext *p_fcc_context;
  FccExecInfo m_fcc_exec_info;

public:
  explicit FuriousExprVisitor(ASTContext *ast_context,
                              FccContext *fcc_context);

  virtual bool VisitExprWithCleanups(ExprWithCleanups *expr);

  virtual bool VisitCXXMemberCallExpr(CXXMemberCallExpr *expr);

  virtual bool VisitCallExpr(CallExpr *func);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class FuriousScriptVisitor : public RecursiveASTVisitor<FuriousScriptVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info
  FccContext *p_fcc_context;

public:
  explicit FuriousScriptVisitor(CompilerInstance* cI,
                                FccContext *fcc_context);

  virtual bool VisitCXXRecordDecl(CXXRecordDecl *record);

  virtual bool VisitLambdaExpr(LambdaExpr *lamnda);

  virtual bool VisitExprWithCleanups(ExprWithCleanups *expr);
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class FccASTVisitor : public RecursiveASTVisitor<FccASTVisitor>
{
private:
  ASTContext *p_ast_context; // used for getting additional AST info
  FuriousScriptVisitor *p_script_visitor;
  FccContext *p_fcc_context;

public:
  explicit FccASTVisitor(CompilerInstance *CI,
                         FccContext *fcc_context);

  virtual bool VisitFunctionDecl(FunctionDecl *func);
};

} // namespace furious

#endif /* ifndef _FURIOUS_CLANGASTVISITOR_H_ */
