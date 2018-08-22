

#include "clang_tools.h"
#include "structs.h"

namespace furious 
{


std::string get_code(SourceManager &sm,
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

/**
 * @brief Extract the execution information from 
 *
 * @param decl
 *
 * @return 
 */
void extract_basic_info(FccExecInfo* exec_info, 
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

  // Check if the declaration is a valid furious api function
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

} /* furious */ 
