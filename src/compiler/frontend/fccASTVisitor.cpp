

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

  const FunctionDecl* func_decl = call->getDirectCallee();

  if( func_decl->getNameAsString().find("operator bool") != std::string::npos)
  {
    // NOTE (Arnau Prat): This is a hack because clang captures lambdas passes as parameters to functions as
    // as CXXMemberCallExpr which I think it makes no sense.
    return true;
  }

  if(func_decl)   
  {                                                                              
    std::string func_name = func_decl->getName();
    QualType ret_type = func_decl->getReturnType().getNonReferenceType();
    const RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();

    // Check if the declaration is a valid furious api call 
    if (ret_type->isStructureOrClassType() &&
        (ret_type->getAsCXXRecordDecl()->getNameAsString() == "QueryBuilder" ||
         ret_type->getAsCXXRecordDecl()->getNameAsString() == "RegisterSystemInfo") &&
        isa<ClassTemplateSpecializationDecl>(ret_decl))
    {
      // Extracting operation type (e.g. foreach, etc.)
      if(func_name == "foreach") 
      {
        m_fcc_exec_info.m_operation_type = FccOperationType::E_FOREACH;
        m_fcc_exec_info.p_ast_context = p_ast_context;
        return process_entry_point(p_ast_context,
                                   p_fcc_context,
                                   &m_fcc_exec_info,
                                   call);
      }

      if(func_name == "has_tag" ) {
        return process_has_tag(p_ast_context,
                                p_fcc_context,
                                &m_fcc_exec_info,
                                call);
      }

      if(func_name == "has_not_tag" ) {
        return process_has_not_tag(p_ast_context,
                                   p_fcc_context,
                                   &m_fcc_exec_info,
                                   call);
      }

      if(func_name == "has_component" ) {
        return process_has_component(p_ast_context,
                                      p_fcc_context,
                                      &m_fcc_exec_info,
                                      call);
      }

      if(func_name == "has_not_component" ) {
        return process_has_not_component(p_ast_context,
                                         p_fcc_context,
                                         &m_fcc_exec_info,
                                         call);
      }

      if(func_name == "filter" ) {
        return process_filter(p_ast_context,
                              p_fcc_context,
                              &m_fcc_exec_info,
                              call);
      }
    } 
  }
  return false;
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
      //func->dump();
      for(auto stmt : func->getBody()->children()) 
      {
        if(isa<Expr>(stmt))
        {
          // Parsing Expressions
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
            if(visitor.m_fcc_exec_info.m_operation_type != FccOperationType::E_UNKNOWN) 
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
        } else 
        {
          // Parsing Declarations
          if(isa<DeclStmt>(stmt))
          {
            const DeclStmt* decl_stmt = cast<DeclStmt>(stmt);
            const Decl* decl = decl_stmt->getSingleDecl();
            if(isa<CXXRecordDecl>(decl))
            {
              // OK
            } else 
            {
              SourceLocation location = decl->getLocStart();
              report_parsing_error(p_ast_context,
                                   p_fcc_context,
                                   location,
                                   FccParsingErrorType::E_UNSUPPORTED_VAR_DECLARATIONS);
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}
}
