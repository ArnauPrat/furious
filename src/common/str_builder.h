

#ifndef _FURIOUS_STR_BUILDER_H_
#define _FURIOUS_STR_BUILDER_H_

#include "types.h"

namespace furious
{

struct str_builder_t 
{
  char*     p_buffer;     //string builder buffer
  uint32_t  m_capacity;   //capacity
  uint32_t  m_pos;        //next position
};

/**
 * \brief Initializes a string builder
 *
 * \param The str_builder
 */
str_builder_t
str_builder_create();

/**
 * \brief Releases a string builder
 *
 * \param The str_builder
 */
void
str_builder_destroy(str_builder_t* str_builder);

/**
 * \brief Appens the string to the string builder
 *
 * \param The str_builder
 * \param str The formatted string to append
 * \param ... The formatted variables
 */
void
str_builder_append(str_builder_t* str_builder,
                   const char* str, 
                   ...);

/**
 * \brief Clears the contents of a string builder
 *
 * \param The str_builder
 */
void
str_builder_clear(str_builder_t* str_builder);


  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_STRING_BUILDER_H_ */
