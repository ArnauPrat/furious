
#include "clang_tools.h"

#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace furious
{

std::string 
get_code(SourceManager &sm,
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
  ASTContext*             p_ast_context;
  std::vector<Dependency> m_dependencies;

  DependenciesVisitor(ASTContext* context) :
  p_ast_context(context)
  {

  }

  bool process_decl(Decl* decl) 
  {
    SourceManager& sm = p_ast_context->getSourceManager();
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
get_dependencies(ASTContext* ast_context, 
                 const Decl* decl)
{
  DependenciesVisitor dep_visitor{ast_context};
  dep_visitor.TraverseDecl(const_cast<Decl*>(decl));
  return dep_visitor.m_dependencies;
}

} /* furious*/
