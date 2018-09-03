

#include <sstream>

#include "structs.h"
#include "fccASTVisitor.h"
#include "transforms.h"
#include "execution_plan.h"
#include "exec_plan_printer.h"

namespace furious 
{

void 
handle_parsing_error(FccContext* context,
                     FccParsingErrorType type,
                     const std::string& filename, 
                     int line,
                     int column
                    )
{
  std::stringstream ss;
  switch(type) 
  {
    case FccParsingErrorType::E_UNKNOWN_ERROR:
      ss << "Unknown error";
      break;
    case FccParsingErrorType::E_UNKNOWN_FURIOUS_OPERATION:
      ss << "Unknown furious operation"; 
      break;
    case FccParsingErrorType::E_INCOMPLETE_FURIOUS_STATEMENT:
      ss << "Incomplete furious statement";
      break;
    case FccParsingErrorType::E_UNSUPPORTED_STATEMENT:
      ss << "Non furious staetment";
      break;
    case FccParsingErrorType::E_UNSUPPORTED_VAR_DECLARATIONS:
      ss << "Non allowed var declaration";
      break;
    case FccParsingErrorType::E_EXPECTED_STRING_LITERAL:
      ss << "Expected String Literal error";
      break;
  }
  ss << " found in " << filename << ":" << line << ":" << column << "\n"; 
  llvm::errs() << ss.str();
}

void 
handle_compilation_error(FccContext* context,
                     FccCompilationErrorType type,
                     const FccOperator* op)
{
  std::stringstream ss;
  switch(type) 
  {
    case FccCompilationErrorType::E_UNKNOWN_ERROR:
      ss << "Unknown error";
      break;
  }
  llvm::errs() << ss.str();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccContext* FccContext_create_and_init() 
{
  FccContext* context = new FccContext();
  FccContext_set_parsing_error_callback(context,
                                handle_parsing_error);

  FccContext_set_compilation_error_callback(context,
                                            handle_compilation_error);
  return context;
}


void 
FccContext_release(FccContext* context)
{
  context->p_pecallback = nullptr;
  context->p_cecallback = nullptr;
  delete context;
}

void 
FccContext_set_parsing_error_callback(FccContext* context,
                                      void (*callback)(FccContext*, 
                                                       FccParsingErrorType,
                                                       const std::string&,
                                                       int32_t,
                                                       int32_t))
{
  context->p_pecallback = callback;
}

void 
FccContext_set_compilation_error_callback(FccContext* context,
                                          void (*callback)(FccContext*, 
                                                           FccCompilationErrorType,
                                                           const FccOperator*))
{
  context->p_cecallback = callback;
}

void 
FccContext_report_parsing_error(FccContext* context,
                                FccParsingErrorType error_type,
                                const std::string& filename,
                                int32_t line,
                                int32_t column)
{
  if(context->p_pecallback) 
  {
    context->p_pecallback(context, 
                         error_type,
                         filename,
                         line,
                         column);
  }
}

void 
FccContext_report_compilation_error(FccContext* context,
                                FccCompilationErrorType error_type,
                                const FccOperator* op)
{
  if(context->p_cecallback) 
  {
    context->p_cecallback(context, 
                          error_type,
                          op);
  }
}

int 
FccContext_run(FccContext* context, 
               CommonOptionsParser& op )
{
  ClangTool tool(op.getCompilations(), op.getSourcePathList());
  int result = tool.buildASTs(context->m_asts);
  if(result != 0)
  {
    return result;
  }

  // Parse and visit all compilation units (furious scripts)
  for(auto& ast : context->m_asts) 
  {
    ASTContext& ast_context = ast->getASTContext();
    FccASTVisitor visitor(&ast_context,
                          context);
    visitor.TraverseDecl(ast_context.getTranslationUnitDecl());
  }

  // Build initial execution plan
  FccExecPlan exec_plan{context};
  for(FccExecInfo& exec_info : context->m_operations)
  {
    FccOperator* next_root = bootstrap_subplan(&exec_info);
    exec_plan.m_roots.push_back(next_root);
    exec_plan.m_asts.push_back(exec_info.p_ast_context);
  }

  ExecPlanPrinter printer;
  printer.traverse(&exec_plan);
  llvm::errs() << printer.m_string_builder.str() << "\n";
  return result;
}

} /* furious */ 
