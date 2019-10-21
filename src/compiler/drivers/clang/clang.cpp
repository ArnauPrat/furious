

#include "clang.h"
#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "ast_visitor.h"
#include "clang_tools.h"
#include "../../frontend/exec_plan.h"
#include "../../../common/common.h"
#include "../../../common/str_builder.h"
#include "../../driver.h"

#include "string.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

namespace furious
{

static cl::OptionCategory fccToolCategory("fcc options");
static cl::opt<std::string> output_file("o", cl::cat(fccToolCategory));
static cl::opt<std::string> include_file("i", cl::cat(fccToolCategory));

static DynArray<std::unique_ptr<ASTUnit>> casts;

static DynArray<QualType*>  p_types;
static DynArray<clang_expr_handler_t*> p_exprs;

QualType*
push_type(QualType qtype)
{
  QualType* ret_qtype = new QualType();
  *ret_qtype = qtype;
  p_types.append(ret_qtype);
  return ret_qtype;
}


clang_expr_handler_t*
push_expr(ASTContext* ctx, 
          const Expr* expr)
{
  clang_expr_handler_t* handler = new clang_expr_handler_t();
  handler->p_context = ctx,
  handler->p_expr = expr;
  p_exprs.append(handler);
  return handler;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fcc_driver_init()
{
}

void
fcc_driver_release()
{
  const uint32_t num_types = p_types.size();
  for(uint32_t i = 0 ; i < num_types; ++i)
  {
    delete p_types[i];
  }

  const uint32_t num_exprs = p_exprs.size();
  for(uint32_t i = 0 ; i < num_exprs; ++i)
  {
    delete p_exprs[i];
  }

  casts.clear();
}

int32_t
fcc_parse_scripts(int argc, 
                  const char** argv,
                  fcc_context_t* fcc_context)
{
  output_file.setInitialValue("furious_generated.cpp");
  include_file.setInitialValue("furious/furious.h");
  CommonOptionsParser op(argc, argv, fccToolCategory);
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
      casts.append(std::move(asts[i]));
    }

#ifndef NDEBUG
    llvm::errs() << "\n";
    llvm::errs() << "Parsing furious scripts" << "\n";
#endif

    // Parse and visit all compilation units (furious scripts)
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
  return result;
}

uint32_t 
fcc_type_name(fcc_type_t type,
              char* buffer,
              uint32_t buffer_length)
{
  QualType aux = *(QualType*)type;
  return get_type_name(aux, buffer, buffer_length);
}

uint32_t
fcc_type_qualified_name(fcc_type_t type,
                        char* buffer,
                        uint32_t buffer_length) 
{
  QualType aux = *(QualType*)type;
  return get_qualified_type_name(aux, 
                          buffer,
                          buffer_length);
}

fcc_access_mode_t
fcc_type_access_mode(fcc_type_t type)
{
  QualType aux = *(QualType*)type;
  return get_access_mode(aux);
}

bool
fcc_type_decl(fcc_type_t type, fcc_decl_t* decl)
{
  *decl = nullptr;
  QualType qtype = *(QualType*)type;
  Decl* clang_decl = get_type_decl(qtype);
  if(clang_decl)
  {
    *decl = clang_decl;
    return true;
  }
  return false;
}

bool
fcc_decl_is_valid(fcc_decl_t decl)
{
  return decl != nullptr;
}

bool
fcc_decl_is_variable_or_struct(fcc_decl_t decl)
{
    return isa<TagDecl>((Decl*)decl);
}

bool
fcc_decl_is_function(fcc_decl_t decl)
{
  bool res = (isa<FunctionDecl>((Decl*)decl) /*&& 
          !cast<FunctionDecl>((Decl*)decl)->isCXXClassMember()*/);
  return res;
}

bool
fcc_decl_is_member(fcc_decl_t decl)
{
  Decl* clang_decl = (Decl*)decl;
  if(isa<TagDecl>(clang_decl))
  {
    if(cast<TagDecl>(clang_decl)->isCXXClassMember())
    {
      return true;
    }
  }

  if(isa<FunctionDecl>(clang_decl))
  {
    if(cast<FunctionDecl>(clang_decl)->isCXXClassMember())
    {
      return true;
    }
  }
  return false;
}

uint32_t
fcc_decl_code(fcc_decl_t decl,
              char* buffer,
              uint32_t buffer_length)
{
  if(fcc_decl_code_available(decl))
  {
    if(fcc_decl_is_function(decl))
    {
      FunctionDecl* func_decl = cast<FunctionDecl>((Decl*)decl);
      if(fcc_decl_is_member(decl))
      {
        str_builder_t str_builder;
        str_builder_init(&str_builder);

        str_builder_append(&str_builder, "auto predicate = [] (");
        auto array = func_decl->parameters();
        char tmp[MAX_TYPE_NAME];
        get_type_name(array[0]->getType(), tmp, MAX_TYPE_NAME);
        str_builder_append(&str_builder, "%s%s", tmp, array[0]->getNameAsString().c_str());

        for(size_t i = 1; i < array.size(); ++i)
        {
          get_type_name(array[i]->getType(), tmp, MAX_TYPE_NAME);
          str_builder_append(&str_builder, ",%s%s", tmp, array[i]->getNameAsString().c_str());
        }
        std::string function_body = get_code(func_decl->getASTContext().getSourceManager(),
                                             func_decl->getSourceRange());
        
        str_builder_append(&str_builder, "%s;\n", function_body.c_str());
        const uint32_t total_length = str_builder.m_pos;
        if(total_length < buffer_length)
        {
          strncpy(buffer, str_builder.p_buffer, buffer_length);
        }
        str_builder_release(&str_builder);
        return total_length;
      } 
      else
      {
        std::string function_code = get_code(func_decl->getASTContext().getSourceManager(),
                                             func_decl->getSourceRange());
        strncpy(buffer, function_code.c_str(), buffer_length);
        return function_code.length();
      }
    }
    else
    {
      Decl* clang_decl = (Decl*)decl;
      const SourceManager& sm = clang_decl->getASTContext().getSourceManager();

      std::string code = get_code(sm,
                                  clang_decl->getSourceRange());

      strncpy(buffer, code.c_str(), buffer_length);
      return code.length();
    }
  }
  return 0;
}

uint32_t
fcc_decl_function_name(fcc_decl_t decl,
                       char* buffer, 
                       uint32_t buffer_length)
{
  if(fcc_decl_is_function(decl))
  {
    if(fcc_decl_is_member(decl))
    {
      char str[] = "predicate";
      strncpy(buffer, str, buffer_length);
      return strlen(str);
    }
    else
    {
      FunctionDecl* clang_decl = cast<FunctionDecl>((Decl*)decl);
      std::string name = clang_decl->getName();
      FURIOUS_COPY_AND_CHECK_STR(buffer, name.c_str(), buffer_length);
      return name.length();
    }
  }

  return 0;
}

bool
fcc_decl_code_available(fcc_decl_t decl)
{
  Decl* clang_decl = (Decl*)decl;
  if(clang_decl->getSourceRange().isValid())
  {
    return true;
  }
  return false;
}


DynArray<Dependency> 
fcc_type_dependencies(fcc_type_t type)
{
  fcc_decl_t decl;
  decl = nullptr;
  if(fcc_type_decl(type, &decl))
  {
    return fcc_decl_dependencies(decl);
  }
  return DynArray<Dependency>();
}


DynArray<Dependency> 
fcc_decl_dependencies(fcc_decl_t decl)
{
  Decl* clang_decl = (Decl*)decl;
  return get_dependencies(clang_decl);
}

uint32_t
fcc_decl_function_num_params(fcc_decl_t decl)
{
  if(fcc_decl_is_function(decl))
  {
    FunctionDecl* clang_decl = cast<FunctionDecl>((Decl*)decl);
    return clang_decl->getNumParams();
  }
  return 0;
}

bool
fcc_decl_function_param_type(fcc_decl_t decl, 
                             uint32_t i, 
                             fcc_type_t* type)
{
  if(fcc_decl_is_function(decl))
  {
    FunctionDecl* clang_decl = cast<FunctionDecl>((Decl*)decl);
    if(i >= clang_decl->getNumParams())
    {
      return false;
    }
    ParmVarDecl* param_decl  = clang_decl->parameters()[i];
    QualType* qtype = push_type(param_decl->getType());
    *type = qtype;
    return true;
  }
  return false;
}

uint32_t
fcc_decl_function_param_name(fcc_decl_t decl, 
                             uint32_t i, 
                             char* buffer,
                             uint32_t buffer_length)
{
  if(fcc_decl_is_function(decl))
  {
    FunctionDecl* clang_decl = cast<FunctionDecl>((Decl*)decl);
    if(i >= clang_decl->getNumParams())
    {
      return 0;
    }
    ParmVarDecl* param_decl  = clang_decl->parameters()[i];
    strncpy(buffer, param_decl->getNameAsString().c_str(), buffer_length);
    return param_decl->getNameAsString().size();
  }
  return 0;
}

bool
fcc_decl_is_same(fcc_decl_t decl_1, fcc_decl_t decl_2)
{
  return decl_1 == decl_2;
}

uint32_t
fcc_expr_code(fcc_expr_t expr,
              char* buffer,
              uint32_t buffer_length)
{
  clang_expr_handler_t* handler = (clang_expr_handler_t*)expr;
  const Expr* clang_expr = handler->p_expr;
  ASTContext* ast_context = handler->p_context;

  const SourceManager& sm = ast_context->getSourceManager();

  std::string code = get_code(sm,
                              clang_expr->getSourceRange());

  strncpy(buffer, code.c_str(), buffer_length);
  return code.length();
}

uint32_t 
get_line_number(const SourceManager& manager,
                const SourceLocation& location) 
{
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getLineNumber(file_id, offset);
}

uint32_t 
get_column_number(const SourceManager& manager,
                  const SourceLocation& location) 
{
  FileID file_id = manager.getFileID(location);
  int offset = manager.getFileOffset(location);
  return manager.getColumnNumber(file_id, offset);
}

uint32_t
get_filename(const SourceManager& sm,
             const SourceLocation& location,
             char* buffer,
             uint32_t buffer_length)
{
  std::string filename = sm.getFilename(location);
  FURIOUS_COPY_AND_CHECK_STR(buffer, filename.c_str(), buffer_length);
  return filename.length();
}

uint32_t 
fcc_expr_code_location(fcc_expr_t expr,
                       char* filename_buffer,
                       uint32_t filename_buffer_length,
                       uint32_t* line,
                       uint32_t* column)
{
  clang_expr_handler_t* handler = (clang_expr_handler_t*)expr;
  Expr* clang_expr = (Expr*)handler->p_expr;
  Decl* decl = get_type_decl(clang_expr->getType());
  const SourceManager& sm = decl->getASTContext().getSourceManager();

  SourceLocation location = clang_expr->getBeginLoc();
  uint32_t filename_length = get_filename(sm, 
                                          location, 
                                          filename_buffer, 
                                          filename_buffer_length);
  *line = get_line_number(sm, location);
  *column = get_column_number(sm, location);

  return filename_length;
}


  
} /* furious */ 
