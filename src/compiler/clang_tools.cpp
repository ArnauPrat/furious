

#include "clang_tools.h"
#include "structs.h"

namespace furious 
{

std::string 
get_code(SourceManager &sm,
         SourceLocation &start,
         SourceLocation &end)
{


  SourceLocation loc = clang::Lexer::getLocForEndOfToken(end,
                                                         0,
                                                         sm,
                                                         LangOptions());
  clang::SourceLocation token_end(loc);
  return std::string(sm.getCharacterData(start),
                     sm.getCharacterData(token_end) - sm.getCharacterData(start));
}

int 
get_line_number(const SourceManager& manager,
                    const SourceLocation& location) {
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getLineNumber(file_id, offset);
}

int 
get_column_number(const SourceManager& manager,
                      const SourceLocation& location) {
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getColumnNumber(file_id, offset);
}

void report_error(ASTContext* context, 
                  const SourceLocation& location,
                  const FccErrorType& type) {

    const SourceManager& sm = context->getSourceManager();
    std::string filename = sm.getFilename(location);
    int32_t line_number = get_line_number(sm, location);
    int32_t column_number = get_column_number(sm, location);
    REPORT_ERROR(type,
                 filename,
                 line_number,
                 column_number
                 );
}


std::vector<QualType> 
get_tmplt_types(const TemplateArgumentList& arg_list) 
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

void process_entry_point(ASTContext* ast_context,
                     FccExecInfo* exec_info,
                     const CallExpr* call)
{

#ifdef DEBUG
  llvm::errs() << "Found Furious Entry Point" << "\n";
#endif

  const FunctionDecl* func_decl = call->getDirectCallee();

  // Extract Basic Components and System from function return type
  QualType ret_type = func_decl->getReturnType();
  const RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();
  const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(ret_decl);
  const TemplateArgumentList& arg_list = tmplt_decl->getTemplateArgs();
  std::vector<QualType> tmplt_types = get_tmplt_types(arg_list);

  exec_info->m_system_type = tmplt_types[0];
  for (size_t i = 1; i < tmplt_types.size(); ++i) 
  {
    exec_info->m_basic_component_types.push_back(tmplt_types[i]);
  }

  // Extract System constructor parameter expressions
  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    exec_info->m_ctor_params.push_back(arg_expr);
  }
}

void process_filter(ASTContext* ast_context,
                    FccExecInfo* exec_info,
                    const CallExpr* call)
{

#ifdef DEBUG
  llvm::errs() << "Found filter" << "\n";
#endif

}

void process_with_tag(ASTContext* ast_context,
                      FccExecInfo* exec_info,
                      const CallExpr* call)
{

#ifdef DEBUG
  llvm::errs() << "Found with_tag" << "\n";
#endif
}

void process_without_tag(ASTContext* ast_context,
                         FccExecInfo* exec_info,
                         const CallExpr* call)
{

#ifdef DEBUG
  llvm::errs() << "Found without_tag" << "\n";
#endif
}

void process_with_component(ASTContext* ast_context,
                            FccExecInfo* exec_info,
                            const CallExpr* call)
{
#ifdef DEBUG
  llvm::errs() << "Found with_component" << "\n";
#endif
}

void process_without_component(ASTContext* ast_context,
                               FccExecInfo* exec_info,
                               const CallExpr* call)
{
#ifdef DEBUG
  llvm::errs() << "Found without_component" << "\n";
#endif
}

/**
 * @brief Extract the execution information from 
 *
 * @param decl
 *
 * @return 
 */
bool extract_exec_info(ASTContext* ast_context,
                        FccExecInfo* exec_info, 
                        const CallExpr* call) 
{

  const FunctionDecl* func_decl = call->getDirectCallee();

  if(func_decl)
  {
    //std::string func_name = func_decl->getQualifiedNameAsString();
    std::string func_name = func_decl->getName();
    QualType ret_type = func_decl->getReturnType().getNonReferenceType();
    const RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();

    // Check if the declaration is a valid furious api call 
    if (ret_type->isStructureOrClassType() &&
        ret_type->getAsCXXRecordDecl()->getNameAsString() == "RegisterSystemInfo" &&
        isa<ClassTemplateSpecializationDecl>(ret_decl))
    {
      // Extracting operation type (e.g. foreach, etc.)
      if(func_name == "register_foreach") 
      {
        exec_info->m_operation_type = OperationType::E_FOREACH;
        process_entry_point(ast_context,
                            exec_info,
                            call);
        return true;
      }

      if(func_name == "with_tag" ) {
        process_with_tag(ast_context,
                         exec_info,
                         call);
        return true;
      }

      if(func_name == "without_tag" ) {
        process_without_tag(ast_context,
                            exec_info,
                            call);
        return true;
      }

      if(func_name == "with_component" ) {
        process_with_component(ast_context,
                               exec_info,
                               call);
        return true;
      }

      if(func_name == "without_component" ) {
        process_without_component(ast_context,
                                  exec_info,
                                  call);
        return true;
      }

      if(func_name == "filter" ) {
        process_filter(ast_context,
                       exec_info,
                       call);
        return true;
      }
    } 
  }
  return false;
}

} /* furious */ 
