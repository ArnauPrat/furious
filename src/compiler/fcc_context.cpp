


#include "fcc_context.h"
#include "frontend/fccASTVisitor.h"
#include "frontend/execution_plan.h"
#include "frontend/exec_plan_printer.h"
#include "backend/codegen.h"

#include "stdlib.h"

#include <stdio.h>
#include <vector>

#include <clang/Tooling/Tooling.h>

#define FCC_CONTEXT_ARRAY_GROWTH_FACTOR 8

namespace furious 
{

FccSystemInfo::FccSystemInfo(FccContext* fcc_context) :
p_fcc_context(fcc_context)
{
}

FccSystemInfo::~FccSystemInfo()
{
}

void
FccSystemInfo::insert_ctor_param(const Expr* param)
{
  m_ctor_params.append(param);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccExecInfo::FccExecInfo(ASTContext* p_ast_context,
                         FccContext* p_fcc_context) :
p_ast_context(p_ast_context),
p_fcc_context(p_fcc_context),
m_operation_type(FccOperationType::E_UNKNOWN),
m_system(p_fcc_context)
{
  
}

FccExecInfo::~FccExecInfo()
{
}

void
FccExecInfo::insert_component_type(const QualType* q_type)
{
  m_basic_component_types.append(*q_type);
}

void
FccExecInfo::insert_has_compponent(const QualType* q_type)
{
  m_has_components.append(*q_type);
}

void
FccExecInfo::insert_has_not_component(const QualType* q_type)
{
  m_has_not_components.append(*q_type);
}

void
FccExecInfo::insert_has_tag(const std::string& tag)
{
  m_has_tags.append(tag);
}

void
FccExecInfo::insert_has_not_tag(const std::string& tag)
{
  m_has_not_tags.append(tag);
}

void
FccExecInfo::insert_filter_func(const FunctionDecl* decl)
{
  p_filter_func.append(decl);
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
  for(uint32_t i = 0; i < p_exec_infos.size(); ++i)
  {
      delete p_exec_infos[i];
  }
}

FccExecInfo*
FccContext::create_exec_info(ASTContext* ast_context)
{
  FccExecInfo* info = new FccExecInfo(ast_context, this);
  p_exec_infos.append(info); 
  return info;
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
                     const FccOperator* op)
{
  StringBuilder str_builder;
  switch(type) 
  {
    case FccCompilationErrorType::E_UNKNOWN_ERROR:
      str_builder.append("Unknown error");
      break;

    case FccCompilationErrorType::E_CYCLIC_DEPENDENCY_GRAPH:
      str_builder.append("Cyclic dependency graph");
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
                             nullptr);
  }

  ExecPlanPrinter printer;
  printer.traverse(&exec_plan);
  llvm::errs() << printer.m_string_builder.p_buffer << "\n";
  generate_code(&exec_plan,
                output_file,
                include_file);
  return result;
}


} /* furious */ 
