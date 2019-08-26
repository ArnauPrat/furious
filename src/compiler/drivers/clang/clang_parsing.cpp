

#include "clang.h"
#include "../common/types.h"
#include "drivers/clang/clang_parsing.h"
#include "drivers/clang/clang_tools.h"
#include "fcc_context.h"
#include <string.h>


namespace furious 
{

void
report_parsing_error(const SourceManager& sm,
                     const SourceLocation& location,
                     fcc_parsing_error_type_t error_type,
                     const char* message)
{

  char filename[2048];
  uint32_t length = get_filename(sm,
                                 location,
                                 filename,
                                 2048);
  FURIOUS_CHECK_STR_LENGTH(length, 2048);
  uint32_t line = get_line_number(sm, location);
  uint32_t column = get_column_number(sm, location);

  fcc_context_report_parsing_error(fcc_parsing_error_type_t::E_UNSUPPORTED_TYPE_MODIFIER,
                                   filename,
                                   line,
                                   column,
                                   message);

}

/**
 * \brief Given a TemplateArgumentList, extract the QualTypes of the template
 * argument types
 *
 * \param arg_list The TemplateArgumentList to extract the types from
 *
 * \return Returns a std::vector with the extracted QualTypes
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
is_global(ASTContext* ast_context,
          QualType qtype
         )
{
  
  if(isa<ClassTemplateSpecializationDecl>(qtype->getAsCXXRecordDecl()))
  {
     if(qtype->getAsRecordDecl()->getNameAsString() == "Global")
     {
       return true;
     }
     else
     {
       const SourceManager& sm = qtype->getAsCXXRecordDecl()->getASTContext().getSourceManager();
       SourceLocation location = qtype->getAsRecordDecl()->getSourceRange().getBegin();
       report_parsing_error(sm,
                            location,
                            fcc_parsing_error_type_t::E_UNSUPPORTED_TYPE_MODIFIER,
                            qtype->getAsRecordDecl()->getNameAsString().c_str());

     }
  }
  return false;
}


bool 
process_match(ASTContext* ast_context,
              fcc_stmt_t*   stmt,
              const CallExpr* call)
{

#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found match",
                   location);
#endif

  const DynArray<fcc_entity_match_t*>& e_matches = stmt->p_entity_matches;
  fcc_entity_match_t* entity_match = e_matches[e_matches.size()-1];

  const FunctionDecl* function_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = function_decl->getTemplateSpecializationArgs();
  DynArray<QualType> tmplt_types = get_tmplt_types(*arg_list);

  // We need to fetch the actual types from the system's signature, since they
  // contain they contain the proper accessmode information. Also, we need to
  // set read the scope from the expand types and propagate to the match types
  // in the system
  uint32_t num_consumed = 0;
  for(uint32_t i = 0; i < e_matches.size() - 1; ++i)
  {
    num_consumed+=e_matches[i]->m_component_types.size();
  }

  DynArray<fcc_component_match_t>& e_match_types = stmt->p_system->m_component_types;
  uint32_t num_system_args = e_match_types.size();
  uint32_t begin = num_system_args - (num_consumed + tmplt_types.size());
  uint32_t end = num_system_args - num_consumed;
  for (size_t i = begin, j = 0; i < end; ++i, ++j) 
  {
    fcc_type_t type = e_match_types[i].m_type;
    bool read_only = e_match_types[i].m_is_read_only;
    bool global = is_global(ast_context, tmplt_types[j]);
    e_match_types[i].m_is_global = global;
    entity_match->m_component_types.append({type,
                                           read_only,
                                           global});
  }
  return true;
}

bool 
process_expand(ASTContext* ast_context,
                fcc_stmt_t*   stmt,
                const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found expand",
                   location);
#endif

  uint32_t num_args = call->getNumArgs();
  if(num_args != 1)
  {
    return false;
  }

  const DynArray<fcc_entity_match_t*>& e_matches = stmt->p_entity_matches;
  fcc_entity_match_t* entity_match = e_matches[e_matches.size()-1];

  const Expr* param_expr = call->getArg(0);
  const uint32_t length = get_string_literal(param_expr,
                                             entity_match->m_ref_name,
                                             MAX_REF_NAME);
  FURIOUS_CHECK_STR_LENGTH(length, MAX_REF_NAME);

  entity_match->m_from_expand = true;

  bool res = process_match(ast_context,
                           stmt,
                           call);

  fcc_entity_match_t * ematch = fcc_entity_match_create();
  stmt->p_entity_matches.append(ematch);
  return res;
}


bool 
process_foreach(ASTContext* ast_context,
                fcc_stmt_t*   stmt,
                const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found foreach",
                   location);
#endif
  static int32_t system_id = 0;
  const FunctionDecl* func_decl = call->getDirectCallee();

  // Initialize entity_match
  fcc_entity_match_t * ematch = fcc_entity_match_create();
  stmt->p_entity_matches.append(ematch);

  // Extract Basic Components and System from function return type
  QualType ret_type = func_decl->getReturnType();
  const RecordDecl* ret_decl = ret_type.getTypePtr()->getAsCXXRecordDecl();
  const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(ret_decl);
  const TemplateArgumentList& arg_list = tmplt_decl->getTemplateArgs();
  DynArray<QualType> tmplt_types = get_tmplt_types(arg_list);

  fcc_system_t* system = new fcc_system_t();
  system->m_id = system_id;
  system->m_system_type.p_handler = push_type(tmplt_types[0]);
  for (size_t i = 1; i < tmplt_types.size(); ++i) 
  {
    QualType type = tmplt_types[i]->getPointeeType();
    bool read_only = get_access_mode(type) == fcc_access_mode_t::E_READ;
    bool global = is_global(ast_context,
                            type);
    QualType* ret_type = push_type(type);
    fcc_type_t fcc_type;
    fcc_type.p_handler = ret_type;
    system->m_component_types.append({fcc_type, read_only, global});
  }

  // Extract System constructor parameter expressions
  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    fcc_expr_t expr;
    expr.p_handler = (void*)push_expr(ast_context, arg_expr);
    system->m_ctor_params.append(expr);
  }
  stmt->p_system = system;
  system_id++;
  return true;
}

bool 
process_set_priority(ASTContext* ast_context,
                     fcc_stmt_t*   stmt,
                     const CallExpr* call)
{
  uint32_t num_args = call->getNumArgs();
  if(num_args != 1)
  {
    return false;
  }

  const Expr* param_expr = call->getArg(0);
  uint32_t priority = get_uint32_literal(param_expr);
  stmt->m_priority = priority;
  return true;
}

bool 
process_set_post_frame(ASTContext* ast_context,
                       fcc_stmt_t*   stmt,
                       const CallExpr* call)
{
  uint32_t num_args = call->getNumArgs();
  if(num_args != 0)
  {
    return false;
  }

  stmt->m_place = fcc_match_place_t::E_POST_FRAME;
  return true;
}

/**
 * \brief Finds and returns the first occurrence of the stmt of type T in a depth first
 * search way.
 *
 * \tparam T The type of expr to find
 * \param expr The expr to start looking at
 *
 * \return Returns the first occurrence of a node of type T (traversed in a DFS way). nullptr otherwise.
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
               fcc_stmt_t* stmt,
               const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found filter",
                   location);
#endif

  const Expr* argument = call->getArg(0);
  const FunctionDecl* func_decl = nullptr;
  const LambdaExpr* lambda = nullptr;
  fcc_entity_match_t* entity_match = stmt->p_entity_matches[stmt->p_entity_matches.size()-1];
  if((lambda = find_first_dfs<LambdaExpr>(argument) ) != nullptr)
  {
    func_decl = lambda->getCallOperator();
    fcc_decl_t fcc_decl;
    fcc_decl.p_handler = (void*)func_decl;
    entity_match->m_filter_func.append(fcc_decl);
  } 

  const DeclRefExpr* decl_ref = nullptr;
  if(func_decl == nullptr && (decl_ref = find_first_dfs<DeclRefExpr>(argument)) != nullptr)
  {
    const ValueDecl* decl = decl_ref->getDecl();
    if(isa<FunctionDecl>(decl))
    {
      func_decl = cast<FunctionDecl>(decl);
      //printf("found function decl: %ld\n", (long)func_decl);
      fcc_decl_t fcc_decl;
      fcc_decl.p_handler = (void*)func_decl;
      entity_match->m_filter_func.append(fcc_decl);
    } 
    else if(isa<VarDecl>(decl))
    {
      const SourceManager& sm = decl->getASTContext().getSourceManager();
      SourceLocation location = call->getSourceRange().getBegin();
      report_parsing_error(sm,
                           location,
                           fcc_parsing_error_type_t::E_UNSUPPORTED_VAR_DECLARATIONS,
                           "");
      return false;
    }
  } 
  return true;
}

bool 
process_has_tag(ASTContext* ast_context,
                 fcc_stmt_t*   stmt,
                 const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found has tag",
                   location);
#endif

  fcc_entity_match_t* entity_match = stmt->p_entity_matches[stmt->p_entity_matches.size()-1];

  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    const clang::StringLiteral* literal = find_first_dfs<clang::StringLiteral>(arg_expr);
    if(literal != nullptr)
    {
      char* buffer = new char[MAX_TAG_NAME];
      strncpy(buffer, literal->getString().str().c_str(), MAX_TAG_NAME);
      FURIOUS_CHECK_STR_LENGTH(strlen(literal->getString().data()), MAX_TAG_NAME);
      entity_match->m_has_tags.append(buffer);
    } 
    else
    {
      const SourceManager& sm = ast_context->getSourceManager();
      SourceLocation location = call->getSourceRange().getBegin();
      report_parsing_error(sm,
                           location,
                           fcc_parsing_error_type_t::E_EXPECTED_STRING_LITERAL,
                           "Non literal types are not allowed.");
      return false;
    }
  }
  return true;
}

bool 
process_has_not_tag(ASTContext* ast_context,
                    fcc_stmt_t* stmt,
                    const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found has not tag",
                   location);
#endif
  fcc_entity_match_t* entity_match = stmt->p_entity_matches[stmt->p_entity_matches.size()-1];
  uint32_t num_args = call->getNumArgs();
  for(uint32_t i = 0; i < num_args; ++i)
  {
    const Expr* arg_expr = call->getArg(i);
    const clang::StringLiteral* literal = find_first_dfs<clang::StringLiteral>(arg_expr);
    if(literal != nullptr)
    {
      char* buffer = new char[MAX_TAG_NAME];
      strncpy(buffer, literal->getString().data(), MAX_TAG_NAME);
      FURIOUS_CHECK_STR_LENGTH(strlen(literal->getString().data()), MAX_TAG_NAME);
      entity_match->m_has_not_tags.append(buffer);
    } 
    else
    {

      const SourceManager& sm = ast_context->getSourceManager();
      SourceLocation location = call->getSourceRange().getBegin();
      report_parsing_error(sm,
                           location,
                           fcc_parsing_error_type_t::E_EXPECTED_STRING_LITERAL,
                           "Non literal types are not allowed");
      return false;
    }
  }
  return true;
}

bool 
process_has_component(ASTContext* ast_context,
                       fcc_stmt_t*  stmt,
                       const CallExpr* call)
{
#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found has component",
                   location);
#endif
  fcc_entity_match_t* entity_match = stmt->p_entity_matches[stmt->p_entity_matches.size()-1];
  const FunctionDecl* func_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = func_decl->getTemplateSpecializationArgs();
  for (uint32_t i = 0; i < arg_list->size(); ++i) 
  {
    const TemplateArgument& arg = arg_list->get(i); 
    fcc_type_t type;
    type.p_handler = push_type(arg.getAsType());
    entity_match->m_has_components.append(type);
  }
  return true;
}

bool 
process_has_not_component(ASTContext* ast_context,
                          fcc_stmt_t*   stmt,
                          const CallExpr* call)
{

#ifndef NDEBUG
  SourceLocation location = call->getSourceRange().getBegin();
  print_debug_info(ast_context,
                   "Found has not component",
                   location);
#endif

  fcc_entity_match_t* entity_match = stmt->p_entity_matches[stmt->p_entity_matches.size()-1];
  const FunctionDecl* func_decl = call->getDirectCallee();
  const TemplateArgumentList* arg_list = func_decl->getTemplateSpecializationArgs();
  for (uint32_t i = 0; i < arg_list->size(); ++i) {
    const TemplateArgument& arg = arg_list->get(i); 
    fcc_type_t type;
    type.p_handler = push_type(arg.getAsType());
    entity_match->m_has_not_components.append(type);
  }
  return true;
}

} /* furious */ 
