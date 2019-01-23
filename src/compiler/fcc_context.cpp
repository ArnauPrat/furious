


#include "fcc_context.h"
#include "frontend/fccASTVisitor.h"
#include "frontend/transforms.h"
#include "frontend/execution_plan.h"
#include "frontend/exec_plan_printer.h"
#include "backend/codegen.h"

#include <stdio.h>
#include <vector>

namespace furious 
{

FccSystemInfo::FccSystemInfo(FccContext* fcc_context) :
p_fcc_context(fcc_context),
m_num_ctor_params(0)
{
}

void
FccSystemInfo::insert_ctor_param(const Expr* param)
{
  if(m_num_ctor_params == FCC_MAX_SYSTEM_CTOR_PARAMS)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_SYSTEM_CTOR_PARAMS,
                                        "",
                                        0,
                                        0);
  }

  m_ctor_params[m_num_ctor_params] = param;
  m_num_ctor_params++;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccExecInfo::FccExecInfo(ASTContext* p_ast_context,
                         FccContext* p_fcc_context) :
p_ast_context(p_ast_context),
p_fcc_context(p_fcc_context),
m_operation_type(FccOperationType::E_UNKNOWN),
m_system(p_fcc_context),
m_num_basic_component_types(0),
m_num_has_components(0),
m_num_has_not_components(0),
m_num_has_tags(0),
m_num_has_not_tags(0),
m_num_func_filters(0)
{
  
}

void
FccExecInfo::insert_component_type(const QualType* q_type)
{
  if(m_num_basic_component_types == FCC_MAX_EXEC_INFO_CTYPES)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_CTYPES,
                                        "",
                                        0,
                                        0);
  }

  m_basic_component_types[m_num_basic_component_types] = *q_type;
  m_num_basic_component_types++;
}

void
FccExecInfo::insert_has_compponent(const QualType* q_type)
{
  if(m_num_has_components == FCC_MAX_EXEC_INFO_HAS)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_HAS,
                                        "",
                                        0,
                                        0);
  }
  m_has_components[m_num_has_components] = *q_type;
  m_num_has_components++;
}

void
FccExecInfo::insert_has_not_component(const QualType* q_type)
{
  if(m_num_has_not_components == FCC_MAX_EXEC_INFO_HAS)
  {
  p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_HAS,
                                      "",
                                      0,
                                      0);
  }
  m_has_not_components[m_num_has_not_components] = *q_type;
  m_num_has_not_components++;
}

void
FccExecInfo::insert_has_tag(const std::string& tag)
{
  if(m_num_has_tags == FCC_MAX_EXEC_INFO_HAS)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_HAS,
                                        "",
                                        0,
                                        0);
  }
  m_has_tags[m_num_has_tags] = tag;
  m_num_has_tags++;
}

void
FccExecInfo::insert_has_not_tag(const std::string& tag)
{
  if(m_num_has_not_tags == FCC_MAX_EXEC_INFO_HAS)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_HAS,
                                        "",
                                        0,
                                        0);
  }

  m_has_not_tags[m_num_has_not_tags] = tag;
  m_num_has_not_tags++;
}

void
FccExecInfo::insert_filter_func(const FunctionDecl* decl)
{
  if(m_num_func_filters == FCC_MAX_EXEC_INFO_FILTER)
  {
    p_fcc_context->report_parsing_error(FccParsingErrorType::E_MAX_EXEC_INFO_FILTER,
                                        "",
                                        0,
                                        0);
  }

  m_filter_func[m_num_func_filters] = decl;
  m_num_func_filters++;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccContext::FccContext(): 
p_pecallback(nullptr),
p_cecallback(nullptr),
m_num_translation_units(0),
m_num_exec_infos(0)
{
  m_exec_infos = new FccExecInfo*[FCC_MAX_OPERATIONS];
  memset(m_exec_infos, '0', sizeof(FccExecInfo*)*FCC_MAX_OPERATIONS);
}

FccContext::~FccContext()
{
  for(uint32_t i = 0; i < m_num_exec_infos; ++i)
  {
    if(m_exec_infos[i] != nullptr)
    {
      delete m_exec_infos[i];
      m_exec_infos[i] = nullptr;
    }
  }

  delete [] m_exec_infos;

}

FccExecInfo*
FccContext::create_exec_info(ASTContext* ast_context)
{
  if(m_num_exec_infos == FCC_MAX_OPERATIONS)
  {
    report_parsing_error(FccParsingErrorType::E_MAX_OPERATION,
                         "",
                         0,
                         0);
  }
  m_exec_infos[m_num_exec_infos] = new FccExecInfo(ast_context, this);
  m_num_exec_infos++;
  return m_exec_infos[m_num_exec_infos-1];
}

void
FccContext::insert_ast_unit(std::unique_ptr<ASTUnit>& unit)
{
  if(m_num_translation_units == FCC_MAX_TRANSLATION_UNITS)
  {
    report_parsing_error(FccParsingErrorType::E_MAX_TRANSLATION_UNIT,
                         "",
                         0,
                         0);
  }
  m_asts[m_num_translation_units] = std::move(unit);
  m_num_translation_units++;
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
    case FccParsingErrorType::E_MAX_SYSTEM_CTOR_PARAMS:
      message = "Maximum number of ctor params achieved";
      break;
    case FccParsingErrorType::E_MAX_EXEC_INFO_CTYPES:
      message = "Maximum number of basic components achieved.";
      break;
    case FccParsingErrorType::E_MAX_EXEC_INFO_HAS:
      message = "Maximum number of has filter types achieved.";
      break;
    case FccParsingErrorType::E_MAX_EXEC_INFO_FILTER:
      message = "Maximum number of function filter types achieved.";
      break;
    case FccParsingErrorType::E_MAX_TRANSLATION_UNIT:
      message = "Maximum number of translation units achieved.";
      break;
    case FccParsingErrorType::E_MAX_OPERATION:
      message = "Maximum number of operations achieved.";
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

  uint32_t BUFFER_SIZE=512;
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%s found in %s:%d:%d\n", message.c_str(), filename.c_str(), line, column);
  llvm::errs() << buffer;
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
  std::vector<std::unique_ptr<ASTUnit>> asts;
  int result = tool.buildASTs(asts);
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
  for(uint32_t i = 0; i < context->m_num_translation_units; ++i) 
  {
    ASTUnit* ast = context->m_asts[i].get();
    ASTContext& ast_context = ast->getASTContext();
    FccASTVisitor visitor(&ast_context,
                          context);
    visitor.TraverseDecl(ast_context.getTranslationUnitDecl());
  }

#ifndef NDEBUG
  llvm::errs() << "\n";
  llvm::errs() << "Building Query Plan" << "\n";
#endif

  // Build initial execution plan
  FccExecPlan exec_plan(context);
  for(uint32_t i = 0; i < context->m_num_exec_infos; ++i)
  {
    FccExecInfo* info = context->m_exec_infos[i];
    FccOperator* next_root = bootstrap_subplan(info);
    exec_plan.insert_root(info->p_ast_context, next_root);
  }

  ExecPlanPrinter printer;
  printer.traverse(&exec_plan);
  llvm::errs() << printer.m_string_builder.str() << "\n";
  generate_code(&exec_plan,
                output_file);
  return result;
}


} /* furious */ 
