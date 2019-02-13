

#ifndef _FURIOUS_STRING_BUILDER_H_
#define _FURIOUS_STRING_BUILDER_H_

#include "types.h"

namespace furious
{

struct StringBuilder
{
  StringBuilder();
  ~StringBuilder();

  void
  append(const char* str, ...);

  void 
  clear();

  char*     p_buffer;     //string builder buffer
  uint32_t  m_capacity;   //capacity
  uint32_t  m_pos;        //next position

};
  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_STRING_BUILDER_H_ */
