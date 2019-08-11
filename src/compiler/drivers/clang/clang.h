

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

/**
 * \brief Gets the line number of a source location
 *
 * \param manager The source manager the source location depends on
 * \param location The source location
 *
 * \return Returns the line represented by the given source location
 */
uint32_t 
get_line_number(const SourceManager& manager,
                const SourceLocation& location);

/**
 * \brief Gets the column number of a source location
 *
 * \param manager The source manager the source location depends on
 * \param location The source location
 *
 * \return Returns the column represented by the given source location
 */
uint32_t 
get_column_number(const SourceManager& manager,
                  const SourceLocation& location);


/**
 * \brief Gets the filename of a source location
 *
 * \param manager The source manager
 * \param location The location
 * \param buffer The buffer to store the filename
 * \param buffer_lenght The length of the buffer
 *
 * \return  The length of the filname
 */
uint32_t
get_filename(const SourceManager& manager,
             const SourceLocation& location,
             char* buffer,
             uint32_t buffer_length);

}

#endif /* ifndef _FURIOUS_CLANG_H_H */
