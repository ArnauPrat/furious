
#include "clang_tools.h"

#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace furious
{

std::string 
get_code(const SourceManager &sm,
         SourceLocation &start,
         SourceLocation &end)
{


  SourceLocation loc = clang::Lexer::getLocForEndOfToken(end,
                                                         0,
                                                         sm,
                                                         LangOptions());
  clang::SourceLocation token_end(loc);
  return std::string(sm.getCharacterData(start),
                     sm.getCharacterData(token_end) - sm.getCharacterData(start));
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
  std::vector<Dependency>       m_dependencies;

  DependenciesVisitor(const ASTContext* context) :
  p_ast_context(context)
  {

  }

  bool process_decl(Decl* decl) 
  {
    const SourceManager& sm = p_ast_context->getSourceManager();
    std::string filename = sm.getFilename(decl->getLocStart());
    std::string file_extension = filename.substr(filename.find_last_of(".") + 1 );

    Dependency dependency;
    if( file_extension != "cpp" && file_extension != "c")
    {
      dependency.m_include_file = filename;
    } else 
    {
      dependency.p_decl = decl;
    }
    m_dependencies.push_back(dependency);
    return true;
  }

  virtual
  bool VisitFunctionDecl(FunctionDecl* decl) 
  {
    return process_decl(decl);
  }

  virtual
  bool VisitCXXRecordDecl(CXXRecordDecl* decl) 
  {
    return process_decl(decl);
  }
};

std::vector<Dependency> 
get_dependencies(const Decl* decl)
{
  const ASTContext& ast_context = decl->getASTContext();
  DependenciesVisitor dep_visitor{&ast_context};
  dep_visitor.TraverseDecl(const_cast<Decl*>(decl));
  return dep_visitor.m_dependencies;
}

std::vector<Dependency> 
get_dependencies(const QualType& type)
{
  const Type* ptr = type.getTypePtrOrNull();
  if(ptr)
  {
    const Decl* decl = nullptr;
    if(type->isAnyPointerType()) 
    {
      decl = type->getPointeeCXXRecordDecl();
    } else 
    {
      decl = type->getAsCXXRecordDecl();
    }
    return get_dependencies(decl);
  }
  return {};//std::vector<Dependency>();
}

std::string 
get_type_name(QualType type)
{
  type.removeLocalConst();
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  return QualType::getAsString(type.split(), policy);
}

std::string 
get_qualified_type_name(QualType type) 
{
  PrintingPolicy policy{{}};
  policy.SuppressTagKeyword = true;
  return QualType::getAsString(type.split(), policy);
}

} /* furious*/
