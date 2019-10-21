
#include "clang_tools.h"
#include "../common/dyn_array.h"

#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <string.h>

namespace furious
{

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
      std::string str_ref = sm.getFilename(decl->getSourceRange().getBegin());

      if(str_ref != "")
      {
        const char* filename = str_ref.c_str();
        const char* extension = strrchr(filename, '.');
        Dependency dependency;
        dependency.m_decl = nullptr;
        if( (extension != nullptr &&
            strcmp(extension, ".cpp") != 0 && 
            strcmp(extension, ".c") != 0 &&
            strcmp(extension, ".inl") != 0))
        {
          FURIOUS_ASSERT(strcmp(filename, "") != 0);
          FURIOUS_COPY_AND_CHECK_STR(dependency.m_include_file, filename, MAX_INCLUDE_PATH_LENGTH);
        } 
        else 
        {
          dependency.m_decl = decl;
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

DynArray<Dependency> 
get_dependencies(const Decl* decl)
{
  const ASTContext& ast_context = decl->getASTContext();
  DependenciesVisitor dep_visitor{&ast_context};
  dep_visitor.TraverseDecl(const_cast<Decl*>(decl));
  return dep_visitor.m_dependencies;
}

uint32_t 
get_type_name(const QualType& type,
              char* buffer,
              uint32_t buffer_length)
{
  QualType aux = type;
  aux.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  std::string str = QualType::getAsString(aux.split(), policy);
  FURIOUS_COPY_AND_CHECK_STR(buffer, str.c_str(), buffer_length);
  return str.size();
}

uint32_t 
get_tagged_type_name(const QualType& type,
                     char* buffer,
                     uint32_t buffer_length)
{
  QualType aux = type;
  aux.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = false;
  std::string str = QualType::getAsString(aux.split(), policy);
  FURIOUS_COPY_AND_CHECK_STR(buffer, str.c_str(), buffer_length);
  return str.size();
}

uint32_t
get_qualified_type_name(const QualType& type,
                     char* buffer,
                     uint32_t buffer_length) 
{
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  std::string str = QualType::getAsString(type.split(), policy);
  FURIOUS_COPY_AND_CHECK_STR(buffer, str.c_str(), buffer_length);
  return str.size();
}

std::string 
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


fcc_access_mode_t
get_access_mode(const QualType& type)
{
  if(type.isConstQualified())
  {
    return fcc_access_mode_t::E_READ;
  }
  return fcc_access_mode_t::E_READ_WRITE;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

class LiteralVisitor : public RecursiveASTVisitor<LiteralVisitor>
{
public:

  std::string str = "";
  uint32_t    uint32 = 0;

  LiteralVisitor() {}

  virtual
  bool VisitStringLiteral(clang::StringLiteral* string_literal)
  {
    str = string_literal->getString(); 
    return true;
  }

  virtual
  bool VisitIntegerLiteral(clang::IntegerLiteral* integer_literal)
  {
    uint32 = integer_literal->getValue().getLimitedValue(); 
    return true;
  }

};

uint32_t
get_string_literal(const Expr* expr,
                   char* buffer,
                   uint32_t buffer_length)
{
  LiteralVisitor visitor;
  visitor.TraverseStmt(const_cast<Expr*>(expr));
  FURIOUS_COPY_AND_CHECK_STR(buffer, visitor.str.c_str(), buffer_length);
  return visitor.str.size();
}

uint32_t
get_uint32_literal(const Expr* expr)
{
  LiteralVisitor visitor;
  visitor.TraverseStmt(const_cast<Expr*>(expr));
  return visitor.uint32;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


Decl*
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

} /* furious*/
