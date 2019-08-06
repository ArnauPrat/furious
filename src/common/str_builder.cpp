


#include "string_builder.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

namespace furious
{

StringBuilder::StringBuilder() :
p_buffer(nullptr),
m_capacity(0),
m_pos(0)
{
  m_capacity = 2048;
  p_buffer = (char*)malloc(m_capacity*sizeof(char));
  p_buffer[0] = 0;
}

StringBuilder::~StringBuilder()
{
  if(p_buffer != nullptr)
  {
    free(p_buffer);
    m_capacity = 0;
  }
}

void
StringBuilder::append(const char* str, ...) 
{
  va_list myargs;
  va_start(myargs, str);

  uint32_t nwritten = 0;
  while( (nwritten = vsnprintf(&p_buffer[m_pos], 
                               m_capacity - m_pos,
                               str, 
                               myargs)) > (m_capacity - m_pos - 1))
         {
         m_capacity +=2048;
         p_buffer = (char*)realloc(p_buffer, sizeof(char)*m_capacity);
         }
  m_pos += nwritten;

  /* Clean up the va_list */
  va_end(myargs);
}

void
StringBuilder::clear()
{
  m_pos = 0;
}


  
} /* furious
 */ 
