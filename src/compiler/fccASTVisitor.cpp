

#include "fccASTVisitor.h"
#include "structs.h"
#include "parsing.h"

#include <clang/Lex/Lexer.h>

namespace furious 
{

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
    report_parsing_error(p_ast_context,
                         p_fcc_context,
                         location,
                         FccParsingErrorType::E_UNKNOWN_FURIOUS_OPERATION);
    return false;
  } 
  return true;
}

bool FuriousExprVisitor::TraverseLambdaBody(LambdaExpr* expr)
{
  // Not to traverse the body of a lambda
  return true;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccASTVisitor::FccASTVisitor(ASTContext *ast_context,
                             FccContext *fcc_context) : 
p_ast_context(ast_context),
p_fcc_context(fcc_context)
{
}

bool FccASTVisitor::VisitFunctionDecl(FunctionDecl *func)
{
  if (func->getNameAsString() == "furious_script")
  {
    if (func->hasBody())
    {
      func->dump();
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

            // We register the execution information obtained with the visitor,
            // after checking it is well-formed.
            if(visitor.m_fcc_exec_info.m_operation_type != OperationType::E_UNKNOWN) 
            {
              p_fcc_context->m_operations.push_back(visitor.m_fcc_exec_info);
            } else 
            {
              SourceLocation location = expr->getLocStart();
              report_parsing_error(p_ast_context,
                           p_fcc_context,
                           location,
                           FccParsingErrorType::E_INCOMPLETE_FURIOUS_STATEMENT);
              return false;
            }
          }
          else 
          {
            SourceLocation location = expr->getLocStart();
            report_parsing_error(p_ast_context,
                                 p_fcc_context,
                                 location,
                                 FccParsingErrorType::E_UNSUPPORTED_STATEMENT);
            return false;
          }
        }
      }
    }
  }
  return true;
}
}
