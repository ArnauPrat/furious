


#include "fcc_context.h"
#include "frontend/fccASTVisitor.h"
#include "frontend/execution_plan.h"
#include "frontend/exec_plan_printer.h"
#include "frontend/parsing.h"
#include "clang_tools.h"
#include "backend/codegen.h"

#include "stdlib.h"

#include <stdio.h>
#include <vector>

#include <clang/Tooling/Tooling.h>

#define FCC_CONTEXT_ARRAY_GROWTH_FACTOR 8

namespace furious 
{

FccSystem::FccSystem(FccContext* fcc_context) :
p_fcc_context(fcc_context)
{
}

FccSystem::~FccSystem()
{
}

void
FccSystem::insert_component_type(const QualType* q_type)
{
  m_component_types.append(*q_type);
}

void
FccSystem::insert_ctor_param(const Expr* param)
{
  m_ctor_params.append(param);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccEntityMatch::FccEntityMatch(FccContext* p_fcc_context) :
p_fcc_context(p_fcc_context),
m_from_expand(false)
{
  
}

FccEntityMatch::~FccEntityMatch()
{
}

void
FccEntityMatch::insert_component_type(const QualType* q_type)
{
  m_basic_component_types.append(*q_type);
}

void
FccEntityMatch::insert_has_compponent(const QualType* q_type)
{
  m_has_components.append(*q_type);
}

void
FccEntityMatch::insert_has_not_component(const QualType* q_type)
{
  m_has_not_components.append(*q_type);
}

void
FccEntityMatch::insert_has_tag(const std::string& tag)
{
  m_has_tags.append(tag);
}

void
FccEntityMatch::insert_has_not_tag(const std::string& tag)
{
  m_has_not_tags.append(tag);
}

void
FccEntityMatch::insert_filter_func(const FunctionDecl* decl)
{
  p_filter_func.append(decl);
}

void
FccEntityMatch::insert_expand(const std::string& str)
{
  m_from_expand = true;
  m_ref_name = str;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccMatch::FccMatch(ASTContext* ast_context,
         FccContext* fcc_context,
         Expr* expr) :
p_ast_context(ast_context),
p_fcc_context(fcc_context),
m_operation_type(FccOperationType::E_UNKNOWN),
m_system(fcc_context),
p_expr(expr)
{
}

FccMatch::~FccMatch()
{
  for(uint32_t i = 0; i < p_entity_matches.size(); ++i)
  {
    delete p_entity_matches[i];
  }
}

FccEntityMatch*
FccMatch::create_entity_match()
{
 FccEntityMatch* entity_match = new FccEntityMatch(p_fcc_context);
 p_entity_matches.append(entity_match);
 return entity_match;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccContext::FccContext(): 
p_pecallback(nullptr),
p_cecallback(nullptr)
{
}

FccContext::~FccContext()
{
  for(uint32_t i = 0; i < p_matches.size(); ++i)
  {
      delete p_matches[i];
  }
}

FccMatch*
FccContext::create_match(ASTContext* ast_context,
                         Expr* expr)
{
  FccMatch* match = new FccMatch(ast_context, 
                                 this, 
                                 expr);
  p_matches.append(match); 
  return match;
}

void
FccContext::insert_ast_unit(std::unique_ptr<ASTUnit>& unit)
{
  p_asts.append(std::move(unit));
}

void
FccContext::insert_using_decl(const UsingDirectiveDecl* using_decl)
{
  p_using_decls.append(using_decl);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void 
handle_parsing_error(FccContext* context,
                     FccParsingErrorType type,
                     const std::string& filename, 
                     int line,
                     int column
                    )
{

  std::string message;
  switch(type) 
  {
    case FccParsingErrorType::E_UNKNOWN_ERROR:
      message = "Unknown error";
      break;
    case FccParsingErrorType::E_UNKNOWN_FURIOUS_OPERATION:
      message = "Unknown furious operation"; 
      break;
    case FccParsingErrorType::E_INCOMPLETE_FURIOUS_STATEMENT:
      message = "Incomplete furious statement";
      break;
    case FccParsingErrorType::E_UNSUPPORTED_STATEMENT:
      message = "Non furious staetment";
      break;
    case FccParsingErrorType::E_UNSUPPORTED_VAR_DECLARATIONS:
      message = "Non allowed var declaration";
      break;
    case FccParsingErrorType::E_EXPECTED_STRING_LITERAL:
      message = "Expected String Literal error";
      break;
  }

  StringBuilder str_builder;
  str_builder.append("%s found in %s:%d:%d\n", message.c_str(), filename.c_str(), line, column);
  llvm::errs() << str_builder.p_buffer;
  abort();
}

void 
handle_compilation_error(FccContext* context,
                         FccCompilationErrorType type,
                         const std::string& err_msg)
{
  StringBuilder str_builder;
  switch(type) 
  {
    case FccCompilationErrorType::E_UNKNOWN_ERROR:
      str_builder.append("Unknown error: %s", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_CYCLIC_DEPENDENCY_GRAPH:
      str_builder.append("Cyclic dependency graph %s", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_INVALID_COLUMN_TYPE:
      str_builder.append("Invalid Column Type: %s", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND:
      str_builder.append("Invalid access mode: %s", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_SYSTEM_INVALID_NUMBER_COMPONENTS:
      str_builder.append("System invalid number of components: %s", err_msg.c_str());
      break;
  }
  llvm::errs() << str_builder.p_buffer;
  abort();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccContext* Fcc_create_context() 
{
  FccContext* context = new FccContext();
  context->set_parsing_error_callback(handle_parsing_error);
  context->set_compilation_error_callback(handle_compilation_error);
  return context;
}

void 
Fcc_release_context(FccContext* context)
{
  context->p_pecallback = nullptr;
  context->p_cecallback = nullptr;
  delete context;
}

void 
FccContext::set_parsing_error_callback(FCC_PARSING_ERROR_CALLBACK callback)
{
  p_pecallback = callback;
}

void 
FccContext::set_compilation_error_callback(FCC_COMP_ERROR_CALLBACK callback)
{
  p_cecallback = callback;
}

void 
FccContext::report_parsing_error(FccParsingErrorType error_type,
                                 const std::string& filename,
                                 int32_t line,
                                 int32_t column)
{
  if(p_pecallback) 
  {
    p_pecallback(this, 
                 error_type,
                 filename,
                 line,
                 column);
  }
  abort();
}

void 
FccContext::report_compilation_error(FccCompilationErrorType error_type,
                                     const std::string& err_msg)
{
  if(p_cecallback) 
  {
    p_cecallback(this, 
                 error_type,
                 err_msg);
  }
  abort();
}

int 
Fcc_run(FccContext* context, 
        CommonOptionsParser& op,
        const std::string& output_file,
        const std::string& include_file)
{
  int result = 0;
  if(op.getSourcePathList().size() > 0)
  {
    ClangTool tool(op.getCompilations(), op.getSourcePathList());
    std::vector<std::unique_ptr<ASTUnit>> asts;
    result = tool.buildASTs(asts);
    if(result != 0)
    {
      return result;
    }

    for(uint32_t i = 0; i < asts.size();++i)
    {
      context->insert_ast_unit(asts[i]);
    }

#ifndef NDEBUG
    llvm::errs() << "\n";
    llvm::errs() << "Parsing furious scripts" << "\n";
#endif

    // Parse and visit all compilation units (furious scripts)
    for(uint32_t i = 0; i < context->p_asts.size(); ++i) 
    {
      ASTUnit* ast = context->p_asts[i].get();
      ASTContext& ast_context = ast->getASTContext();
      FccASTVisitor visitor(&ast_context,
                            context);
      visitor.TraverseDecl(ast_context.getTranslationUnitDecl());
    }

#ifndef NDEBUG
    llvm::errs() << "\n";
    llvm::errs() << "Building Query Plan" << "\n";
#endif
  }

  // Build initial execution plan
  FccExecPlan exec_plan(context);

  if(!exec_plan.bootstrap())
  {
    handle_compilation_error(context, 
                             FccCompilationErrorType::E_CYCLIC_DEPENDENCY_GRAPH, 
                             "");
  }

  ExecPlanPrinter printer;
  printer.traverse(&exec_plan);
  llvm::errs() << printer.m_string_builder.p_buffer << "\n";
  generate_code(&exec_plan,
                output_file,
                include_file);
  return result;
}

void
Fcc_validate(const FccMatch* match)
{

  if(match->m_operation_type == FccOperationType::E_UNKNOWN)
  {
    SourceLocation location = match->p_expr->getLocStart();
    report_parsing_error(match->p_ast_context,
                        match->p_fcc_context,
                        location,
                        FccParsingErrorType::E_UNSUPPORTED_STATEMENT);
  }

  uint32_t num_matches = match->p_entity_matches.size();
  if( num_matches == 0)
  {
    SourceLocation location = match->p_expr->getLocStart();
    report_parsing_error(match->p_ast_context,
                        match->p_fcc_context,
                        location,
                        FccParsingErrorType::E_UNSUPPORTED_STATEMENT);
  }

  DynArray<bool> allow_writes;

  FccEntityMatch* e_match = match->p_entity_matches[num_matches - 1];
  for(uint32_t i = 0; i < e_match->m_basic_component_types.size(); ++i)
  {
    allow_writes.append(true);
  }

  for(int32_t i = num_matches - 2; i >= 0; --i)
  {
    e_match = match->p_entity_matches[i];
    for(uint32_t j = 0; j < e_match->m_basic_component_types.size(); ++j)
    {
      allow_writes.append(false);
    }
  }

  if(allow_writes.size() != match->m_system.m_component_types.size())
  {
      StringBuilder str_builder;
      str_builder.append("Match: %u, System: %u", allow_writes.size(), match->m_system.m_component_types.size());
      match->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_SYSTEM_INVALID_NUMBER_COMPONENTS,
                                                     str_builder.p_buffer);
  }

  for(uint32_t i = 0; i < match->m_system.m_component_types.size(); ++i)
  {
    QualType type = match->m_system.m_component_types[i];
    if(allow_writes[i] == false &&
       (get_access_mode(type) == FccAccessMode::E_WRITE ||
        get_access_mode(type) == FccAccessMode::E_READ_WRITE
       ))
    {
      StringBuilder str_builder;
      str_builder.append("\"%s\" access mode after expand must be read-only", get_type_name(type).c_str());
      match->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                                     str_builder.p_buffer);
    }
  }

}

} /* furious */ 
