


#include "str_builder.h"
#include "memory/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


void
fdb_str_builder_init(struct fdb_str_builder_t* str_builder)
{
  str_builder->m_capacity = 2048;
  str_builder->p_buffer = (char*) mem_alloc(fdb_get_global_mem_allocator(), 
                                           1, 
                                           sizeof(char)*str_builder->m_capacity, 
                                           FDB_NO_HINT);
  str_builder->p_buffer[0] = '\0';
  str_builder->m_pos = 0;
  str_builder->p_buffer[str_builder->m_pos] = 0;
}

void fdb_str_builder_release(struct fdb_str_builder_t* str_builder)
{
  if(str_builder->p_buffer != NULL)
  {
    mem_free(fdb_get_global_mem_allocator(), 
             str_builder->p_buffer);
    str_builder->m_capacity = 0;
  }
}

void
fdb_str_builder_append(struct fdb_str_builder_t* str_builder,
                   const char* str, 
                   ...) 
{
  va_list myargs;
  va_start(myargs, str);

  uint32_t nwritten = 0;
  while( (nwritten = vsnprintf(&str_builder->p_buffer[str_builder->m_pos], 
                               str_builder->m_capacity - str_builder->m_pos,
                               str, 
                               myargs)) > (str_builder->m_capacity - str_builder->m_pos - 1))
  {
    uint32_t new_capacity = str_builder->m_capacity + 2048;
    char* new_buffer = (char*) mem_alloc(fdb_get_global_mem_allocator(), 
                                         1, 
                                         sizeof(char)*new_capacity, 
                                         -1);
    memcpy(new_buffer, 
           str_builder->p_buffer, 
           sizeof(char)*str_builder->m_capacity);
    mem_free(fdb_get_global_mem_allocator(), 
             str_builder->p_buffer);
    str_builder->p_buffer = new_buffer;
    str_builder->m_capacity = new_capacity;
  }
  str_builder->m_pos += nwritten;
  str_builder->p_buffer[str_builder->m_pos] = '\0';

  /* Clean up the va_list */
  va_end(myargs);
}

void
fdb_str_builder_clear(struct fdb_str_builder_t* str_builder)
{
  str_builder->m_pos = 0;
}

