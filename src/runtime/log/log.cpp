

#include "log.h"

namespace furious
{

log_func_t m_log_function = nullptr;

void
set_log_function(log_func_t log_function)
{
  m_log_function = log_function;
}
  
} /* furious */ 
