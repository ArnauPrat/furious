

#include "fccASTVisitor.h"
#include "structs.h"

namespace furious {

/**
 * @brief Gets the code between start and end SourceLocations.
 *
 * @param sm The source manager to get the code from
 * @param start The start location of the code
 * @param end The end location of the code.
 *
 * @return Returns a string with the code in the expecified source location
 * range
 */
std::string get_code(SourceManager &sm,
                     SourceLocation &start,
                     SourceLocation &end)
{

  clang::SourceLocation token_end(clang::Lexer::getLocForEndOfToken(end,
                                                                    0,
                                                                    sm,
                                                                    LangOptions()));
  return std::string(sm.getCharacterData(start),
                     sm.getCharacterData(token_end) - sm.getCharacterData(start));
}

/**
 * @brief Given a TemplateArgumentList, extract the QualTypes of the template
 * argument types
 *
 * @param arg_list The TemplateArgumentList to extract the types from
 *
 * @return Returns a std::vector with the extracted QualTypes
 */
std::vector<QualType> get_tmplt_types(const TemplateArgumentList& arg_list) 
{
  std::vector<QualType> ret;

  for (auto arg : arg_list.asArray())
  {
    switch(arg.getKind()) {
    case TemplateArgument::ArgKind::Null:
      break;
    case TemplateArgument::ArgKind::Type:
      ret.push_back(arg.getAsType());
      break;
    case TemplateArgument::ArgKind::Declaration:
      break;
    case TemplateArgument::ArgKind::NullPtr:
      break;
    case TemplateArgument::ArgKind::Integral:
      break;
    case TemplateArgument::ArgKind::Template:
      break;
    case TemplateArgument::ArgKind::TemplateExpansion:
      break;
    case TemplateArgument::ArgKind::Expression:
      break;
    case TemplateArgument::ArgKind::Pack:
      for (auto arg2 : arg.getPackAsArray())
      {
        ret.push_back(arg2.getAsType());
      }
      break;
    }
  }
  return ret;
}

/**
 * @brief Extract the execution information from 
 *
 * @param decl
 *
 * @return 
 */
static void extract_basic_info(FccExecInfo* exec_info, 
                                 const FunctionDecl* func_decl) 
{
  std::string func_name = func_decl->getQualifiedNameAsString();
  QualType ret_type = func_decl->getReturnType();
  exec_info->m_operation_type = OperationType::E_UNKNOWN;
  if(func_name == "furious::register_foreach" ) {
    exec_info->m_operation_type = OperationType::E_FOREACH;
  }

  if(exec_info->m_operation_type == OperationType::E_UNKNOWN) {
    // trigger error;
    return;
  }

  // Check if the declaration is a register system entry point
  if (func_decl->isTemplateInstantiation() &&
      func_decl->getTemplatedKind() == FunctionDecl::TemplatedKind::TK_FunctionTemplateSpecialization &&
      ret_type->isStructureOrClassType() &&
      ret_type->getAsCXXRecordDecl()->getNameAsString() == "RegisterSystemInfo")
  {
    RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();
    const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(ret_decl);
    const TemplateArgumentList& arg_list = tmplt_decl->getTemplateArgs();
    std::vector<QualType> tmplt_types = get_tmplt_types(arg_list);

    exec_info->m_system_type = tmplt_types[0];
    for (size_t i = 1; i < tmplt_types.size(); ++i) {
      exec_info->m_basic_component_types.push_back(tmplt_types[i]);
    }
  } else {
    // trigger error
    return;
  }
  return;
}


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


} /* furious */ 
