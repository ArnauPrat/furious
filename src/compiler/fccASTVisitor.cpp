

#include "fccASTVisitor.h"
#include "structs.h"
#include "clang_tools.h"

#include <clang/Lex/Lexer.h>

namespace furious {


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FuriousExprVisitor::FuriousExprVisitor(ASTContext *ast_context,
                                       FccContext *fcc_context) : 
p_ast_context(ast_context), 
  p_fcc_context(fcc_context)
{
}

bool FuriousExprVisitor::VisitExprWithCleanups(ExprWithCleanups *expr)
{
  expr->dump(llvm::errs());
  return true;
}

bool FuriousExprVisitor::VisitCXXMemberCallExpr(CXXMemberCallExpr *expr)
{
  llvm::errs() << "CXXMemberCallExpr found"
    << "\n";
  CXXMethodDecl *method_decl = expr->getMethodDecl();
  CXXRecordDecl *record_decl = expr->getRecordDecl();

  std::string method_name = method_decl->getNameAsString();
  std::string record_name = record_decl->getNameAsString();

  llvm::errs() << record_name << " " << method_name << "\n";

  return true;

}

bool FuriousExprVisitor::VisitCallExpr(CallExpr *func)
{
  FunctionDecl *func_decl = func->getDirectCallee();
  if (func_decl)
  {
    extract_basic_info(&m_fcc_exec_info,
                       func_decl);
    llvm::errs() << m_fcc_exec_info.m_system_type.getAsString() << "\n";
    for(auto type : m_fcc_exec_info.m_basic_component_types) {
      llvm::errs() << type.getAsString() << "\n";
    }
  } else {
    // Trigger error of unsupported statatment?
  }
  return true;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FuriousScriptVisitor::FuriousScriptVisitor(CompilerInstance *CI,
                                           FccContext *fcc_context) : 
p_ast_context(&(CI->getASTContext())),
  p_fcc_context(fcc_context)
{
}

bool FuriousScriptVisitor::VisitCXXRecordDecl(CXXRecordDecl *record)
{
  return true;
}

bool FuriousScriptVisitor::VisitLambdaExpr(LambdaExpr *lamnda)
{
  llvm::errs() << "Found lamnda expr" << "\n";
  return true;
}

bool FuriousScriptVisitor::VisitExprWithCleanups(ExprWithCleanups *expr)
{
  FuriousExprVisitor visitor(p_ast_context,
                             p_fcc_context);

  visitor.TraverseExprWithCleanups(expr);
  llvm::errs() << "expr with cleanups found" << "\n";
  return true;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccASTVisitor::FccASTVisitor(CompilerInstance *CI,
                             FccContext *fcc_context) : p_ast_context(&(CI->getASTContext())),
  p_script_visitor(new FuriousScriptVisitor(CI, fcc_context)),
  p_fcc_context(fcc_context)
{
}

bool FccASTVisitor::VisitFunctionDecl(FunctionDecl *func)
{
  if (func->getNameAsString() == "furious_script")
  {
    if (func->hasBody())
    {
      p_script_visitor->TraverseCompoundStmt(cast<CompoundStmt>(func->getBody()));
    }
  }
  return true;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

  // override the constructor in order to pass CI
FccASTConsumer::FccASTConsumer(CompilerInstance *cI,
                               FccContext *fcc_context)
  : visitor(new FccASTVisitor(cI, fcc_context)) // initialize the visitor
{
}

// override this to call our ExampleVisitor on the entire source file
void
FccASTConsumer::HandleTranslationUnit(ASTContext &context)
{
  // context.getTranslationUnitDecl()->dump(llvm::errs());
  /* we can use ASTContext to get the TranslationUnitDecl, which is
     a single Decl that collectively represents the entire source file */
  visitor->TraverseDecl(context.getTranslationUnitDecl());
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

std::unique_ptr<ASTConsumer>
FccFrontendAction::CreateASTConsumer(CompilerInstance &cI, 
                                     StringRef file)
{
 return std::make_unique<FccASTConsumer>(&cI, fcc_get_context()); // pass CI pointer to ASTConsumer
}

} /* furious */ 
