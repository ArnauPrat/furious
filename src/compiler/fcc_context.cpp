


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

namespace furious 
{

FccContext* p_fcc_context = nullptr;

FccMatch::~FccMatch()
{
  for(uint32_t i = 0; i < p_entity_matches.size(); ++i)
  {
    delete p_entity_matches[i];
  }

  if(p_system != nullptr)
  {
    delete p_system;
  }
}

FccEntityMatch* 
FccMatch::create_entity_match()
{
  FccEntityMatch* e_match = new FccEntityMatch(); 
  p_entity_matches.append(e_match);
  return e_match;
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
  FccMatch* match = new FccMatch();
  match->p_ast_context = ast_context;
  match->p_expr = expr;
  p_matches.append(match); 
  return match;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void 
handle_parsing_error(FccParsingErrorType type,
                     const std::string& filename, 
                     int line,
                     int column,
                     const std::string& message
                    )
{

  StringBuilder str_builder;
  switch(type) 
  {
    case FccParsingErrorType::E_UNKNOWN_ERROR:
      str_builder.append("Unknown error");
      break;
    case FccParsingErrorType::E_UNKNOWN_FURIOUS_OPERATION:
      str_builder.append("Unknown furious operation"); 
      break;
    case FccParsingErrorType::E_INCOMPLETE_FURIOUS_STATEMENT:
      str_builder.append("Incomplete furious statement ");
      break;
    case FccParsingErrorType::E_UNSUPPORTED_STATEMENT:
      str_builder.append("Non furious staetment");
      break;
    case FccParsingErrorType::E_UNSUPPORTED_VAR_DECLARATIONS:
      str_builder.append("Non allowed var declaration");
      break;
    case FccParsingErrorType::E_UNSUPPORTED_TYPE_MODIFIER:
      str_builder.append("Unsupported type modifier");
      break;
    case FccParsingErrorType::E_EXPECTED_STRING_LITERAL:
      str_builder.append("Expected String Literal error");
      break;
    case FccParsingErrorType::E_NO_ERROR:
      str_builder.append("No Error. This should never be reported");
      break;
  }

  str_builder.append(" found in %s:%d:%d. %s\n", filename.c_str(), line, column, message.c_str());
  llvm::errs() << str_builder.p_buffer;
  abort();
}

void 
handle_compilation_error(FccCompilationErrorType type,
                         const std::string& err_msg)
{
  StringBuilder str_builder;
  switch(type) 
  {
    case FccCompilationErrorType::E_UNKNOWN_ERROR:
      str_builder.append("Unknown error: %s\n", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_CYCLIC_DEPENDENCY_GRAPH:
      str_builder.append("Cyclic dependency graph %s\n", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_INVALID_COLUMN_TYPE:
      str_builder.append("Invalid Column Type: %s\n", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND:
      str_builder.append("Invalid access mode: %s\n", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_SYSTEM_INVALID_NUMBER_COMPONENTS:
      str_builder.append("System invalid number of components: %s\n", err_msg.c_str());
      break;
    case FccCompilationErrorType::E_NO_ERROR:
      str_builder.append("No Error. This should never be reported");
      break;
  }
  llvm::errs() << str_builder.p_buffer;
  abort();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void Fcc_create_context() 
{
  p_fcc_context = new FccContext();
  p_fcc_context->set_parsing_error_callback(handle_parsing_error);
  p_fcc_context->set_compilation_error_callback(handle_compilation_error);
}

void 
Fcc_release_context()
{
  delete p_fcc_context;
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
                                 int32_t column,
                                 const std::string& message) const
{
  if(p_pecallback != nullptr) 
  {
    p_pecallback(error_type,
                 filename,
                 line,
                 column,
                 message);
  }
  abort();
}

void 
FccContext::report_compilation_error(FccCompilationErrorType error_type,
                                     const std::string& err_msg) const
{
  if(p_cecallback != nullptr) 
  {
    p_cecallback(error_type,
                 err_msg);
  }
  abort();
}

int 
Fcc_run(CommonOptionsParser& op,
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
      p_fcc_context->p_asts.append(std::move(asts[i]));
    }

#ifndef NDEBUG
    llvm::errs() << "\n";
    llvm::errs() << "Parsing furious scripts" << "\n";
#endif

    // Parse and visit all compilation units (furious scripts)
    const DynArray<std::unique_ptr<ASTUnit>>& casts = p_fcc_context->p_asts;
    for(uint32_t i = 0; i < casts.size(); ++i) 
    {
      ASTUnit* ast = casts[i].get();
      ASTContext& ast_context = ast->getASTContext();
      FccASTVisitor visitor(&ast_context);
      visitor.TraverseDecl(ast_context.getTranslationUnitDecl());
    }

#ifndef NDEBUG
    llvm::errs() << "\n";
    llvm::errs() << "Building Query Plan" << "\n";
#endif
  }

  // Build execution plan
  DynArray<const FccMatch*> frame_matches;
  uint32_t num_matches = p_fcc_context->p_matches.size();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
    const FccMatch* next_match = p_fcc_context->p_matches[i];
    if(next_match->m_place == FccMatchPlace::E_FRAME)
    {
      frame_matches.append(next_match);
    }
  }

  FccExecPlan* exec_plan;
  FccCompilationErrorType err = create_execplan(frame_matches.buffer(), 
                                                frame_matches.size(), 
                                                &exec_plan);
  if(err != FccCompilationErrorType::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  // Build post execution plan
  DynArray<const FccMatch*> post_matches;
  num_matches = p_fcc_context->p_matches.size();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
    const FccMatch* next_match = p_fcc_context->p_matches[i];
    if(next_match->m_place == FccMatchPlace::E_POST_FRAME)
    {
      post_matches.append(next_match);
    }
  }

  FccExecPlan* post_exec_plan;
  err = create_execplan(post_matches.buffer(), 
                        post_matches.size(), 
                        &post_exec_plan);

  if(err != FccCompilationErrorType::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  llvm::errs() << "Printing Frame execplan" << "\n";
  {
    ExecPlanPrinter printer;
    DynArray<uint32_t> seq = get_valid_exec_sequence(exec_plan);
    const uint32_t num_in_seq = seq.size();
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      printer.traverse(&exec_plan->m_subplans[seq[j]]);
    }
    llvm::errs() << printer.m_string_builder.p_buffer << "\n";
  }

  {
    ExecPlanPrinter post_printer;
    llvm::errs() << "Printing PostFrame execplan" << "\n"; 
    DynArray<uint32_t> seq = get_valid_exec_sequence(post_exec_plan);
    const uint32_t num_in_seq = seq.size();
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      post_printer.traverse(&post_exec_plan->m_subplans[seq[j]]);
    }
    llvm::errs() << post_printer.m_string_builder.p_buffer << "\n";
  }
  
  generate_code(exec_plan,
                post_exec_plan,
                output_file,
                include_file);

  destroy_execplan(exec_plan);
  destroy_execplan(post_exec_plan);
  return result;
}

void
Fcc_validate(const FccMatch* match)
{
  if(match->m_operation_type == FccOperationType::E_UNKNOWN)
  {
    SourceLocation location = match->p_expr->getSourceRange().getBegin();
    report_parsing_error(match->p_ast_context,
                         location,
                         FccParsingErrorType::E_UNSUPPORTED_STATEMENT,
                         "");
  }

  const DynArray<FccEntityMatch*>& e_matches = match->p_entity_matches;
  uint32_t num_matches = e_matches.size();
  if( num_matches == 0 )
  {
    SourceLocation location = match->p_expr->getSourceRange().getBegin();
    report_parsing_error(match->p_ast_context,
                         location,
                         FccParsingErrorType::E_UNSUPPORTED_STATEMENT,
                         "At least one match must exist");
  }

  if( num_matches > 2)
  {
    SourceLocation location = match->p_expr->getSourceRange().getBegin();
    report_parsing_error(match->p_ast_context,
                         location,
                         FccParsingErrorType::E_UNSUPPORTED_STATEMENT,
                         "More than 2 expands found. The maximum number of expands per statement is 1");
  }

  DynArray<bool> allow_writes;
  DynArray<bool> allow_globals;

  FccEntityMatch* e_match = e_matches[num_matches - 1];
  for(uint32_t i = 0; i < e_match->m_match_types.size(); ++i)
  {
    allow_writes.append(true);
    allow_globals.append(true);
  }

  for(int32_t i = num_matches - 2; i >= 0; --i)
  {
    e_match = e_matches[i];
    for(uint32_t j = 0; j < e_match->m_match_types.size(); ++j)
    {
      allow_writes.append(false);
      allow_globals.append(false);
    }
  }

  const DynArray<FccMatchType>& match_types = match->p_system->m_match_types;
  if(allow_writes.size() != match_types.size())
  {
      StringBuilder str_builder;
      str_builder.append("Match: %u, System: %u", allow_writes.size(), match_types.size());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_SYSTEM_INVALID_NUMBER_COMPONENTS,
                                              str_builder.p_buffer);
  }

  bool all_globals = true;
  for(uint32_t i = 0; i < match_types.size(); ++i)
  {
    const FccMatchType* match_type = &match_types[i];
    if(allow_writes[i] == false &&
       !match_type->m_is_read_only)
    {
      StringBuilder str_builder;
      str_builder.append("\"%s\" access mode after expand must be read-only", get_type_name(match_type->m_type).c_str());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                                     str_builder.p_buffer);
    }

    if(allow_globals[i] == false &&
       match_type->m_is_global)
    {
      StringBuilder str_builder;
      str_builder.append("\"%s\" scope modifier after expand cannot be global", get_type_name(match_type->m_type).c_str());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                                     str_builder.p_buffer);
    }
    all_globals &= match_type->m_is_global;
  }

  if(!all_globals)
  {
    for(uint32_t i = 0; i < match_types.size(); ++i)
    {
      const FccMatchType* match_type = &match_types[i];
      if(!match_type->m_is_read_only && match_type->m_is_global)
      {
        StringBuilder str_builder;
        str_builder.append("\"%s\" read-write access mode on globals is only allowed \"global-only\" systems", get_type_name(match_type->m_type).c_str());
        p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                                       str_builder.p_buffer);
      }
    }
  }

}

} /* furious */ 
