
#include "clang_tools.h"
#include "../common/dyn_array.h"

#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <string.h>

namespace furious
{

/*const Decl*
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
*/


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
  strncpy(buffer, str.c_str(), buffer_length-1);
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
  strncpy(buffer, str.c_str(), buffer_length-1);
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
  strncpy(buffer, str.c_str(), buffer_length-1);
  return str.size();
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
