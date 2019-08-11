

#include "clang.h"
#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "ast_visitor.h"
#include "../../frontend/exec_plan.h"
#include "../../../common/common.h"
#include "../../../common/str_builder.h"
#include "../../driver.h"

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


static Decl*
get_type_decl(const QualType& type)
{
  SplitQualType unqual = type.getSplitUnqualifiedType();
  const clang::Type* t = unqual.Ty;

  if(t->isAnyPointerType()) 
  {
    t = t->getPointeeType().getTypePtr();
  } 

  /*if(t->isCanonicalUnqualified())
  {
    return t->getAsTagDecl();
  }
  else
  {
  */
    const TypedefType* tdef = t->getAs<TypedefType>();
    if(tdef)
    {
      return tdef->getDecl();
    }

    if(t->isFunctionType())
    {
      return t->getAsTagDecl();
    }

    if(t->isRecordType())
    {
      return t->getAsCXXRecordDecl();
    }

  //}
    
  return t->getAsTagDecl();
}

static uint32_t 
get_type_name(const QualType& type,
              char* buffer,
              uint32_t buffer_length)
{
  QualType aux = type;
  aux.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  std::string str = QualType::getAsString(aux.split(), policy);
  strncpy(buffer, str.c_str(), buffer_length);
  return str.size();
}

static std::string 
get_code(const SourceManager &sm,
         SourceRange range)
{


  SourceLocation loc = clang::Lexer::getLocForEndOfToken(range.getEnd(),
                                                         0,
                                                         sm,
                                                         LangOptions());
  clang::SourceLocation token_end(loc);
  return std::string(sm.getCharacterData(range.getBegin()),
                     sm.getCharacterData(token_end) - sm.getCharacterData(range.getBegin()));
}

/**
 * @brief Visitor used to extract the dependencies from a declaration, either
 * as include files or in-place declarations.
 */
class DependenciesVisitor : public RecursiveASTVisitor<DependenciesVisitor>
{
public:
  const ASTContext*             p_ast_context;
  DynArray<Dependency>          m_dependencies;

  DependenciesVisitor(const ASTContext* context) :
  p_ast_context(context)
  {

  }

  bool process_decl(Decl* decl) 
  {
    const SourceManager& sm = p_ast_context->getSourceManager();
    if(decl->getSourceRange().isValid())
    {
      std::string filename = sm.getFilename(decl->getSourceRange().getBegin());
      std::string file_extension = filename.substr(filename.find_last_of(".") + 1 );

      if(filename != "")
      {
        Dependency dependency;
        if( file_extension != "cpp" && 
            file_extension != "c" &&
            file_extension != "inl")
        {
          FURIOUS_ASSERT(filename != "");
          dependency.m_include_file = filename;
        } 
        else 
        {
          dependency.m_decl.p_handler = decl;
        }
        m_dependencies.append(dependency);
      }
    }
    return true;
  }

  virtual
  bool VisitExpr(Expr* expr)
  {
    if(expr->getReferencedDeclOfCallee())
    {
      return process_decl(expr->getReferencedDeclOfCallee());
    }
    const QualType& qtype = expr->getType();
    Decl* vdecl = get_type_decl(qtype);
    if(vdecl)
    {
      return process_decl(vdecl);
    }
    return true;
  }

  virtual
  bool VisitDecl(Decl* decl) 
  {
    return process_decl(decl);
  }
};


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
  QualType aux = *(QualType*)type.p_handler;
  return get_type_name(aux, buffer, buffer_length);
}

uint32_t
fcc_type_qualified_name(fcc_type_t type,
                        char* buffer,
                        uint32_t buffer_length) 
{
  QualType aux = *(QualType*)type.p_handler;
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  std::string str = QualType::getAsString(aux.split(), policy);
  strncpy(buffer, str.c_str(), buffer_length);
  return str.size();
}

fcc_access_mode_t
fcc_type_access_mode(fcc_type_t type)
{
  QualType aux = *(QualType*)type.p_handler;
  if(aux.isConstQualified())
  {
    return fcc_access_mode_t::E_READ;
  }
  return fcc_access_mode_t::E_READ_WRITE;
}

bool
fcc_type_decl(fcc_type_t type, fcc_decl_t* decl)
{

  QualType qtype = *(QualType*)type.p_handler;
  Decl* clang_decl = get_type_decl(qtype);
  if(clang_decl)
  {
    decl->p_handler = clang_decl;
    return true;
  }
  return false;
}

bool
fcc_decl_is_valid(fcc_decl_t decl)
{
  return decl.p_handler != nullptr;
}

bool
fcc_decl_is_variable_or_struct(fcc_decl_t decl)
{
    return isa<TagDecl>((Decl*)decl.p_handler);
}

bool
fcc_decl_is_function(fcc_decl_t decl)
{
  return (isa<FunctionDecl>((Decl*)decl.p_handler) && 
          !cast<FunctionDecl>((Decl*)decl.p_handler)->isCXXClassMember());
}

bool
fcc_decl_is_lambda(fcc_decl_t decl)
{
  Decl* clang_decl = ((Decl*)decl.p_handler);
  if(isa<FunctionDecl>(clang_decl))
  {
    const FunctionDecl* func_decl = cast<FunctionDecl>(clang_decl);
    if(func_decl->isCXXClassMember())
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
      FunctionDecl* func_decl = cast<FunctionDecl>((Decl*)decl.p_handler);
      if(fcc_decl_is_lambda(decl))
      {
        str_builder_t str_builder;
        str_builder_init(&str_builder);

        str_builder_append(&str_builder, "auto predicate = [] (");
        auto array = func_decl->parameters();
        char tmp[MAX_TYPE_NAME];
        uint32_t length = get_type_name(array[0]->getType(), tmp, MAX_TYPE_NAME);
        FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);
        str_builder_append(&str_builder, "%s%s", tmp, array[0]->getNameAsString().c_str());

        for(size_t i = 1; i < array.size(); ++i)
        {
          const uint32_t length = get_type_name(array[i]->getType(), tmp, MAX_TYPE_NAME);
          FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);
          str_builder_append(&str_builder, ",%s%s", tmp, array[i]->getNameAsString().c_str());
        }
        uint32_t code_length = 4096;
        char * code_buffer = new char[code_length];
        length = 0;
        while((length = fcc_decl_code(decl, code_buffer, code_length) ) >= code_length)
        {
          delete [] code_buffer;
          code_length *= 2;
          code_buffer = new char[code_length];
        }
        str_builder_append(&str_builder, "%s;\n", code_buffer);
        delete [] code_buffer;
        
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
        return 0;
      }
    }
    else
    {

      Decl* clang_decl = (Decl*)decl.p_handler;
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
      FunctionDecl* clang_decl = cast<FunctionDecl>((Decl*)decl.p_handler);
      std::string name = clang_decl->getName();
      strncpy(buffer, name.c_str(), buffer_length);
      return name.length();
  }

  if(fcc_decl_is_lambda(decl))
  {
    char str[] = "predicate";
    strncpy(buffer, str, buffer_length);
    return strlen(str);
  }

  return 0;
}

bool
fcc_decl_code_available(fcc_decl_t decl)
{
  Decl* clang_decl = (Decl*)decl.p_handler;
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
  if(fcc_type_decl(type, &decl))
  {
    return fcc_decl_dependencies(decl);
  }
  return DynArray<Dependency>();
}

DynArray<Dependency> 
fcc_decl_dependencies(fcc_decl_t decl)
{
  Decl* clang_decl = (Decl*)decl.p_handler;
  const ASTContext& ast_context = clang_decl->getASTContext();
  DependenciesVisitor dep_visitor{&ast_context};
  dep_visitor.TraverseDecl(const_cast<Decl*>(clang_decl));
  return dep_visitor.m_dependencies;
}

bool
fcc_decl_is_same(fcc_decl_t decl_1, fcc_decl_t decl_2)
{
  return decl_1.p_handler == decl_2.p_handler;
}

uint32_t
fcc_expr_code(fcc_expr_t expr,
              char* buffer,
              uint32_t buffer_length)
{
  clang_expr_handler_t* handler = (clang_expr_handler_t*)expr.p_handler;
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
  strncpy(buffer, filename.c_str(), buffer_length);
  return filename.length();
}


uint32_t 
fcc_expr_code_location(fcc_expr_t expr,
                       char* filename_buffer,
                       uint32_t filename_buffer_length,
                       uint32_t* line,
                       uint32_t* column)
{
  clang_expr_handler_t* handler = (clang_expr_handler_t*)expr.p_handler;
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
