
#include "ast_visitor.h"
#include "../../fcc_context.h"
#include "clang_parsing.h"


FuriousExprVisitor::FuriousExprVisitor(ASTContext *ast_context) : 
p_ast_context(ast_context), 
p_fcc_stmt(nullptr)
{
}

fcc_stmt_t*
FuriousExprVisitor::parse_expression(Expr* expr)
{
  p_fcc_stmt = new fcc_stmt_t;
  *p_fcc_stmt = fcc_stmt_init();
  p_fcc_stmt->m_expr = expr;
  fcc_stmt_ptr_array_append(&p_fcc_context->m_stmts, &p_fcc_stmt); 
  this->TraverseStmt(expr);
  return p_fcc_stmt;
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
        (ret_type->getAsCXXRecordDecl()->getNameAsString() == "MatchQueryBuilder" ||
         ret_type->getAsCXXRecordDecl()->getNameAsString() == "RegisterSystemInfo") &&
        isa<ClassTemplateSpecializationDecl>(ret_decl))
    {
      // Extracting operation type (e.g. foreach, etc.)
      if(func_name == "foreach") 
      {
        p_fcc_stmt->m_operation_type = fcc_operation_type_t::E_FOREACH;
        return process_foreach(p_ast_context,
                               p_fcc_stmt,
                               call);
      }

      if(func_name == "set_priority") 
      {
        return process_set_priority(p_ast_context,
                                       p_fcc_stmt,
                                       call);
      }

      if(func_name == "set_post_frame") 
      {
        return process_set_post_frame(p_ast_context,
                                      p_fcc_stmt,
                                      call);
      }

      if(func_name == "has_tag" ) 
      {
        return process_has_tag(p_ast_context,
                               p_fcc_stmt,
                               call);
      }

      if(func_name == "has_not_tag" ) 
      {
        return process_has_not_tag(p_ast_context,
                                   p_fcc_stmt,
                                   call);
      }

      if(func_name == "has_component" ) 
      {
        return process_has_component(p_ast_context,
                                     p_fcc_stmt,
                                     call);
      }

      if(func_name == "has_not_component" ) 
      {
        return process_has_not_component(p_ast_context,
                                         p_fcc_stmt,
                                         call);
      }

      if(func_name == "filter" ) 
      {
        return process_filter(p_ast_context,
                              p_fcc_stmt,
                              call);
      }

      if(func_name == "match" ) 
      {
        bool res = process_match(p_ast_context,
                                 p_fcc_stmt,
                                 call);
        return res;
      }

      if(func_name == "expand" ) 
      {
        bool res = process_expand(p_ast_context,
                                  p_fcc_stmt,
                                  call);
        return res;
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

FccASTVisitor::FccASTVisitor(ASTContext *ast_context) : 
p_ast_context(ast_context)
{
}


bool FccASTVisitor::VisitFunctionDecl(FunctionDecl *func)
{

  if (func->getNameAsString() == "furious_script")
  {
    //func->dump();
    if (func->hasBody())
    {
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
            FuriousExprVisitor visitor(p_ast_context);

            fcc_stmt_t* stmt = visitor.parse_expression(expr);

            // We register the execution information obtained with the visitor,
            // after checking it is well-formed.
            fcc_validate(stmt);
          }
          else 
          {
            SourceLocation location = expr->getSourceRange().getBegin();
            report_parsing_error(p_ast_context->getSourceManager(),
                                 location,
                                 fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                 "Unknown expression type");
            return false;
          }
        } 
        else 
        {
          // Parsing Declarations
          if(isa<DeclStmt>(stmt))
          {
            const DeclStmt* decl_stmt = cast<DeclStmt>(stmt);
            const Decl* decl = decl_stmt->getSingleDecl();
            if(isa<CXXRecordDecl>(decl))
            {
              // OK
            } 
            else if(isa<UsingDirectiveDecl>(decl))
            {
              // OK
              const UsingDirectiveDecl* using_decl = cast<UsingDirectiveDecl>(decl);
              fcc_decl_t fcc_decl;
              fcc_decl = (void*)using_decl;
              fcc_decl_array_append(&p_fcc_context->m_using_decls, &fcc_decl);
            }
            else 
            {
              SourceLocation location = decl->getSourceRange().getBegin();
              report_parsing_error(p_ast_context->getSourceManager(),
                                   location,
                                   fcc_parsing_error_type_t::E_UNSUPPORTED_VAR_DECLARATIONS,
                                   "");
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}
