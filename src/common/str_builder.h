

#ifndef _FDB_STR_BUILDER_H_
#define _FDB_STR_BUILDER_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif


struct fdb_str_builder_t 
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
void
fdb_str_builder_init(struct fdb_str_builder_t* str_builder);

/**
 * \brief Releases a string builder
 *
 * \param The str_builder
 */
void
fdb_str_builder_release(struct fdb_str_builder_t* str_builder);

/**
 * \brief Appens the string to the string builder
 *
 * \param The str_builder
 * \param str The formatted string to append
 * \param ... The formatted variables
 */
void
fdb_str_builder_append(struct fdb_str_builder_t* str_builder,
                   const char* str, 
                   ...);

/**
 * \brief Clears the contents of a string builder
 *
 * \param The str_builder
 */
void
fdb_str_builder_clear(struct fdb_str_builder_t* str_builder);


#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_STRING_BUILDER_H_ */
