

#ifndef _FURIOUS_CLANG_H_H
#define _FURIOUS_CLANG_H_H 

#include "../../../common/types.h"


namespace clang
{
  class Expr;
  class ASTContext;
  class QualType;
  class SourceManager;
  class SourceLocation;
}

using namespace clang;

namespace furious
{

struct clang_expr_handler_t
{
  const Expr*       p_expr;
  ASTContext*  p_context;
};

QualType*
push_type(QualType qtype);

clang_expr_handler_t*
push_expr(ASTContext* ctx, 
          const Expr* expr);

}

#endif /* ifndef _FURIOUS_CLANG_H_H */
