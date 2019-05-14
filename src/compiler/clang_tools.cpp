
#include "clang_tools.h"
#include "../common/dyn_array.h"

#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace furious
{

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

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * @brief Visitor used to extract the dependencies from a declaration, either
 * as include files or in-place declarations.
 */
class DependenciesVisitor : public RecursiveASTVisitor<DependenciesVisitor>
{
public:
  const ASTContext*             p_ast_context;
  DynArray<Dependency>       m_dependencies;

  DependenciesVisitor(const ASTContext* context) :
  p_ast_context(context)
  {

  }

  bool process_decl(const Decl* decl) 
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
          dependency.m_include_file = filename;
        } 
        else 
        {
          dependency.p_decl = decl;
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
    const Decl* vdecl = get_type_decl(qtype);
    if(vdecl)
    {
      return process_decl(vdecl);
    }
    return true;
  }

  virtual
  bool VisitDecl(Decl* decl) 
  {
    //printf("PROCESSING Declaration\n");
    return process_decl(decl);
  }
};

const Decl*
get_type_decl(const QualType& type)
{

  //printf("PROCESING %s\n", get_type_name(type).c_str());
  SplitQualType unqual = type.getSplitUnqualifiedType();
  const clang::Type* t = unqual.Ty;

  if(t->isAnyPointerType()) 
  {
    t = t->getPointeeType().getTypePtr();
    //printf("Is Pointer Type %s\n", get_type_name(type).c_str());
  } 

  if(t->isCanonicalUnqualified())
  {
    //printf("Processed Type %s\n", get_type_name(type).c_str());
    return t->getAsTagDecl();
  }
  else
  {
    const TypedefType* tdef = t->getAs<TypedefType>();
    if(tdef)
    {
      //printf("Non Cannonical but found typedef %s\n", get_type_name(type).c_str());
      return tdef->getDecl();
    }

    if(t->isFunctionType())
    {
      //printf("Non Cannonical Function Type %s\n", get_type_name(type).c_str());
      return t->getAsTagDecl();
    }
  }

  //printf("NOT PROCESSED Type %s\n", get_type_name(type).c_str());
    
  return nullptr;
}

DynArray<Dependency> 
get_dependencies(const Decl* decl)
{
  const ASTContext& ast_context = decl->getASTContext();
  DependenciesVisitor dep_visitor{&ast_context};
  dep_visitor.TraverseDecl(const_cast<Decl*>(decl));
  return dep_visitor.m_dependencies;
}

DynArray<Dependency> 
get_dependencies(const QualType& type)
{
  const Decl* decl = get_type_decl(type);
  if(decl)
  {
    return get_dependencies(decl);
  }
  return DynArray<Dependency>();
}

std::string 
get_type_name(const QualType& type)
{
  QualType aux = type;
  aux.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  return QualType::getAsString(aux.split(), policy);
}

std::string 
get_tagged_type_name(const QualType& type)
{
  QualType aux = type;
  aux.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = false;
  return QualType::getAsString(aux.split(), policy);
}

std::string 
get_qualified_type_name(const QualType& type) 
{
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  return QualType::getAsString(type.split(), policy);
}


FccAccessMode
get_access_mode(const QualType& type)
{
  if(type.isConstQualified())
  {
    return FccAccessMode::E_READ;
  }
  return FccAccessMode::E_READ_WRITE;
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

std::string
get_string_literal(const Expr* expr)
{
  LiteralVisitor visitor;
  visitor.TraverseStmt(const_cast<Expr*>(expr));
  return visitor.str;
}

uint32_t
get_uint32_literal(const Expr* expr)
{
  LiteralVisitor visitor;
  visitor.TraverseStmt(const_cast<Expr*>(expr));
  return visitor.uint32;
}

} /* furious*/
