

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

bool FuriousExprVisitor::VisitCallExpr(CallExpr* call)
{
  if(!extract_exec_info(p_ast_context,
                        &m_fcc_exec_info,
                        call))
  {
    SourceLocation location = call->getLocStart();
    report_error(p_ast_context,
                 location,
                 FccErrorType::E_UNKNOWN_FURIOUS_OPERATION);
    return false;
  } 
  return true;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccASTVisitor::FccASTVisitor(CompilerInstance *CI,
                             FccContext *fcc_context) : 
p_ast_context(&(CI->getASTContext())),
p_fcc_context(fcc_context)
{
}

bool FccASTVisitor::VisitFunctionDecl(FunctionDecl *func)
{
  if (func->getNameAsString() == "furious_script")
  {
    if (func->hasBody())
    {
      for(auto stmt : func->getBody()->children()) 
      {
        if(isa<Expr>(stmt))
        {
          Expr* expr = cast<Expr>(stmt);
          QualType expr_type = expr->getType();
          const RecordDecl* ret_decl = expr_type->getAsCXXRecordDecl();
          // We need to translate those expressions that evaluate into a
          // "RegisterSystemInfo" type
          if(expr_type->isStructureOrClassType() &&
             expr_type->getAsCXXRecordDecl()->getNameAsString() == "RegisterSystemInfo" &&
             isa<ClassTemplateSpecializationDecl>(ret_decl) )
          {
            FuriousExprVisitor visitor(p_ast_context,
                                       p_fcc_context);

            visitor.TraverseStmt(expr);

            // TODO: Register the information extracted using the visitor
          }
          else 
          {
            SourceLocation location = expr->getLocStart();
            report_error(p_ast_context,
                         location,
                         FccErrorType::E_UNSUPPORTED_STATEMENT);
          }
        }
      }
      //p_script_visitor->TraverseCompoundStmt(cast<CompoundStmt>(func->getBody()));
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
