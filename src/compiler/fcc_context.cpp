

#include <sstream>

#include "fcc_context.h"
#include "frontend/fccASTVisitor.h"
#include "frontend/transforms.h"
#include "frontend/execution_plan.h"
#include "frontend/exec_plan_printer.h"
#include "backend/codegen.h"

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
}

void 
FccContext::report_compilation_error(FccCompilationErrorType error_type,
                                     const FccOperator* op)
{
  if(p_cecallback) 
  {
    p_cecallback(this, 
                 error_type,
                 op);
  }
}

int 
Fcc_run(FccContext* context, 
        CommonOptionsParser& op,
        const std::string& output_file)
{
  ClangTool tool(op.getCompilations(), op.getSourcePathList());
  int result = tool.buildASTs(context->m_asts);
  if(result != 0)
  {
    return result;
  }

#ifdef DEBUG
  llvm::errs() << "\n";
  llvm::errs() << "Parsing furious scripts" << "\n";
#endif

  // Parse and visit all compilation units (furious scripts)
  for(auto& ast : context->m_asts) 
  {
    ASTContext& ast_context = ast->getASTContext();
    FccASTVisitor visitor(&ast_context,
                          context);
    visitor.TraverseDecl(ast_context.getTranslationUnitDecl());
  }

#ifdef DEBUG
  llvm::errs() << "\n";
  llvm::errs() << "Building Query Plan" << "\n";
#endif

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
  generate_code(&exec_plan,
                output_file);
  return result;
}


} /* furious */ 
