

#include "log.h"
#include <stddef.h>


fdb_log_func_t m_log_function = NULL;

void
set_log_function(fdb_log_func_t log_function)
{
  m_log_function = log_function;
}
