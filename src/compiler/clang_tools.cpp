
#include "clang_tools.h"
#include <clang/Lex/Lexer.h>

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


} /* furious
 */ 
