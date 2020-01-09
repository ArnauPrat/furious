


#ifndef _FURIOUS_REFLECTION_H_
#define _FURIOUS_REFLECTION_H_ value

#include "../../common/platform.h"
#include "../../common/dyn_array.h"
#include "../../common/refcount_ptr.h"

namespace furious
{

struct ReflField;
struct ReflData;

enum class ReflType
{
  E_BOOL,
  E_CHAR,
  E_UINT8,
  E_UINT16,
  E_UINT32,
  E_UINT64,
  E_INT8,
  E_INT16,
  E_INT32,
  E_INT64,
  E_FLOAT,
  E_DOUBLE,
  E_CHAR_POINTER,
  E_STD_STRING,
  E_POINTER,
  E_STRUCT,
  E_UNION,
  E_UNKNOWN,
  E_NUM_TYPES,
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct ReflData
{
  char                  m_type_name[FCC_MAX_TYPE_NAME];
  DynArray<ReflField>   m_fields;
  bool                  m_is_union;
};


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct ReflField
{
  char                        m_name[FCC_MAX_FIELD_NAME];
  ReflType                    m_type;
  size_t                      m_offset;
  bool                        m_anonymous;
  RefCountPtr<ReflData>       p_strct_type;
};


} /* furious */ 

#endif /* ifndef _FURIOUS_REFLECTION_H_ */
