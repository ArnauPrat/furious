
#ifndef _FDB_LOG_H_
#define _FDB_LOG_H_ value

#include "../../common/str_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum fdb_log_level_t
{
  E_INFO,
  E_WARNING,
  E_ERROR
} fdb_log_level_t;

typedef void (*fdb_log_func_t) (fdb_log_level_t, const char*);  // hint with the block id of this allocation. -1 indicates no hint

extern fdb_log_func_t m_log_function;

#define FURIOUS_LOG(level, msg, ...) \
  if(m_log_function) \
  { \
    str_builder_t str_builder = str_builder_create();\
    str_builder_append(&str_builder, msg, __VA_ARGS__)\
    m_log_function(level, str_builder.p_buffer);\
    str_builder_destroy(&str_builder);\
  }
  
void
set_log_function(fdb_log_func_t log_function);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_LOG_H_ */
