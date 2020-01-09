
#ifndef __FURIOUS_LOG_H_
#define _FURIOUS_LOG_H_ value

#include "../../common/str_builder.h"

namespace furious
{

enum log_level_t
{
  E_INFO,
  E_WARNING,
  E_ERROR
};


using log_func_t = void (*) (log_level_t, 
                             const char*);  // hint with the block id of this allocation. -1 indicates no hint

extern log_func_t m_log_function;

#define FURIOUS_LOG(level, msg, ...) \
  if(m_log_function) \
  { \
    str_builder_t str_builder = str_builder_create();\
    str_builder_append(&str_builder, msg, __VA_ARGS__)\
    m_log_function(level, str_builder.p_buffer);\
    str_builder_destroy(&str_builder);\
  }
  
void
set_log_function(log_func_t log_function);
  
} /* furious */ 
#endif /* ifndef _FURIOUS_LOG_H_ */
