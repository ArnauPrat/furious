

#include "parsing.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious 
{

/**
 * @brief Gets the line number of a source location
 *
 * @param manager The source manager the source location depends on
 * @param location The source location
 *
 * @return Returns the line represented by the given source location
 */
int 
get_line_number(const SourceManager& manager,
                const SourceLocation& location) 
{
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getLineNumber(file_id, offset);
}

/**
 * @brief Gets the column number of a source location
 *
 * @param manager The source manager the source location depends on
 * @param location The source location
 *
 * @return Returns the column represented by the given source location
 */
int 
get_column_number(const SourceManager& manager,
                  const SourceLocation& location) 
{
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getColumnNumber(file_id, offset);
}

void 
report_parsing_error(ASTContext* ast_context, 
                     FccContext* fcc_context,
                     const SourceLocation& location,
                     const FccParsingErrorType& type) 
{
  const SourceManager& sm = ast_context->getSourceManager();
  std::string filename = sm.getFilename(location);
  int32_t line_number = get_line_number(sm, location);
  int32_t column_number = get_column_number(sm, location);
  fcc_context->report_parsing_error(type,
                                    filename,
                                    line_number,
                                    column_number);
}

/**
 * @brief Given a TemplateArgumentList, extract the QualTypes of the template
 * argument types
 *
 * @param arg_list The TemplateArgumentList to extract the types from
 *
 * @return Returns a std::vector with the extracted QualTypes
 */
DynArray<QualType> 
get_tmplt_types(const TemplateArgumentList& arg_list) 
{
  DynArray<QualType> ret;

  for (auto arg : arg_list.asArray())
  {
    switch(arg.getKind()) {
      case TemplateArgument::ArgKind::Null:
        break;
      case TemplateArgument::ArgKind::Type:
        ret.append(arg.getAsType());
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
          ret.append(arg2.getAsType());
        }
        break;
    }
  }
  return ret;
}

inline
void 
print_debug_info(const ASTContext* ast_context,
                 const std::string& str,
                 SourceLocation& location)
{
  const SourceManager& sm = ast_context->getSourceManager();
  std::string filename = sm.getFilename(location);
  llvm::errs() << str << " at " 
               << filename << ":" 
               << get_line_number(sm, location) 
               << get_column_number(sm, location)<< "\n";
}

bool 
process_match(ASTContext* ast_context,
                FccContext* fcc_context,
                FccMatch*   fcc_match,
                const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found match",
                   location);
#endif

  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];

  const FunctionDecl* function_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = function_decl->getTemplateSpecializationArgs();
  DynArray<QualType> tmplt_types = get_tmplt_types(*arg_list);

  for (size_t i = 1; i < tmplt_types.size(); ++i) 
  {
    QualType type = tmplt_types[i]->getPointeeType();
    entity_match->insert_component_type(&type);
  }
  return true;
}

bool 
process_expand(ASTContext* ast_context,
                FccContext* fcc_context,
                FccMatch*   fcc_match,
                const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found expand",
                   location);
#endif

  uint32_t num_args = call->getNumArgs();
  if(num_args != 1)
  {
    return false;
  }

  const Expr* param_expr = call->getArg(0);
  std::string str = get_string_literal(param_expr);
  fcc_match->insert_expand(str);

  bool res = process_match(ast_context,
                           fcc_context,
                           fcc_match,
                           call);
  fcc_match->create_entity_match();
  return res;
}


bool 
process_foreach(ASTContext* ast_context,
                FccContext* fcc_context,
                FccMatch*   fcc_match,
                const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found foreach",
                   location);
#endif
  static int32_t system_id = 0;
  const FunctionDecl* func_decl = call->getDirectCallee();

  // Initialize entity_match
  fcc_match->create_entity_match();

  // Extract Basic Components and System from function return type
  QualType ret_type = func_decl->getReturnType();
  const RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();
  const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(ret_decl);
  const TemplateArgumentList& arg_list = tmplt_decl->getTemplateArgs();
  DynArray<QualType> tmplt_types = get_tmplt_types(arg_list);

  FccSystem* system = &fcc_match->m_system;
  system->m_id = system_id;
  system->m_system_type = tmplt_types[0];
  for (size_t i = 1; i < tmplt_types.size(); ++i) 
  {
    QualType type = tmplt_types[i]->getPointeeType();
    system->insert_component_type(&type);
  }

  // Extract System constructor parameter expressions
  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    system->insert_ctor_param(arg_expr);
  }
  system_id++;
  return true;
}

/**
 * @brief Finds and returns the first occurrence of the stmt of type T in a depth first
 * search way.
 *
 * @tparam T The type of expr to find
 * @param expr The expr to start looking at
 *
 * @return Returns the first occurrence of a node of type T (traversed in a DFS way). nullptr otherwise.
 */
template<typename T>
const T* find_first_dfs(const Stmt* expr)
{
  for(const Stmt* stmt : expr->children())
  {
    if(isa<Expr>(stmt))
    {
      if(isa<T>(stmt))
      {
        return cast<T>(stmt);
      }
      else
      {
        const T* l_expr = find_first_dfs<T>(stmt);
        if(l_expr != nullptr)
        {
          return l_expr;
        }
      }
    } 
  }
  return nullptr;
}

bool 
process_filter(ASTContext* ast_context,
               FccContext* fcc_context,
               FccMatch* fcc_match,
               const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found filter",
                   location);
#endif

  const Expr* argument = call->getArg(0);
  const FunctionDecl* func_decl = nullptr;
  const LambdaExpr* lambda = nullptr;
  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];
  if((lambda = find_first_dfs<LambdaExpr>(argument) ) != nullptr)
  {
    func_decl = lambda->getCallOperator();
    entity_match->insert_filter_func(func_decl);
  } 

  const DeclRefExpr* decl_ref = nullptr;
  if(func_decl == nullptr && (decl_ref = find_first_dfs<DeclRefExpr>(argument)) != nullptr)
  {
    const ValueDecl* decl = decl_ref->getDecl();
    if(isa<FunctionDecl>(decl))
    {
      func_decl = cast<FunctionDecl>(decl);
      entity_match->insert_filter_func(func_decl);
    } else if(isa<VarDecl>(decl))
    {
      // TODO: Report error via callback
      SourceLocation location = call->getLocStart();
      report_parsing_error(ast_context,
                           fcc_context,
                           location,
                           FccParsingErrorType::E_UNSUPPORTED_VAR_DECLARATIONS);
      return false;
    }
  } 
  return true;
}

bool 
process_has_tag(ASTContext* ast_context,
                 FccContext* fcc_context,
                 FccMatch*   fcc_match,
                 const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found has tag",
                   location);
#endif

  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];

  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    const clang::StringLiteral* literal = find_first_dfs<clang::StringLiteral>(arg_expr);
    if(literal != nullptr)
    {
      entity_match->insert_has_tag(literal->getString());
    } 
    else
    {

      SourceLocation location = call->getLocStart();
      report_parsing_error(ast_context,
                           fcc_context,
                           location,
                           FccParsingErrorType::E_EXPECTED_STRING_LITERAL);
      return false;
    }
  }
  return true;
}

bool 
process_has_not_tag(ASTContext* ast_context,
                    FccContext* fcc_context,
                    FccMatch* fcc_match,
                    const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found has not tag",
                   location);
#endif
  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];
  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    const clang::StringLiteral* literal = find_first_dfs<clang::StringLiteral>(arg_expr);
    if(literal != nullptr)
    {
      entity_match->insert_has_not_tag(literal->getString());
    } 
    else
    {

      SourceLocation location = call->getLocStart();
      report_parsing_error(ast_context,
                           fcc_context,
                           location,
                           FccParsingErrorType::E_EXPECTED_STRING_LITERAL);
      return false;
    }
  }
  return true;
}

bool 
process_has_component(ASTContext* ast_context,
                       FccContext* fcc_contex,
                       FccMatch*  fcc_match,
                       const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found has component",
                   location);
#endif
  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];
  const FunctionDecl* func_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = func_decl->getTemplateSpecializationArgs();
  for (uint32_t i = 0; i < arg_list->size(); ++i) 
  {
    const TemplateArgument& arg = arg_list->get(i); 
    QualType type = arg.getAsType();
    entity_match->insert_has_compponent(&type);
  }
  return true;
}

bool 
process_has_not_component(ASTContext* ast_context,
                          FccContext* fcc_contex,
                          FccMatch*   fcc_match,
                          const CallExpr* call)
{

#ifndef NDEBUG
  SourceLocation location = call->getLocStart();
  print_debug_info(ast_context,
                   "Found has not component",
                   location);
#endif

  FccEntityMatch* entity_match = fcc_match->p_entity_matches[fcc_match->p_entity_matches.size()-1];
  const FunctionDecl* func_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = func_decl->getTemplateSpecializationArgs();
  for (uint32_t i = 0; i < arg_list->size(); ++i) {
    const TemplateArgument& arg = arg_list->get(i); 
    QualType type = arg.getAsType();
    entity_match->insert_has_not_component(&type);
  }
  return true;
}

} /* furious */ 
