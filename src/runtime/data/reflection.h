


#ifndef _FDB_REFLECTION_H_
#define _FDB_REFLECTION_H_

#include "../../common/platform.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/btree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum fdb_mtype_t 
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
} fdb_mtype_t;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef struct fdb_mstruct_t 
{
  char                  m_type_name[FDB_MAX_TABLE_NAME];
  struct fdb_mfield_t*  p_fields[FDB_MAX_COMPONENT_FIELDS];
  uint32_t              m_nfields;
  bool                  m_is_union;
} fdb_mstruct_t;


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef struct fdb_mfield_t
{
  char                        m_name[FCC_MAX_FIELD_NAME];
  fdb_mtype_t                 m_type;
  size_t                      m_offset;
  bool                        m_anonymous;
  fdb_mstruct_t*              p_strct_type;
} fdb_mfield_t;


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


typedef struct fdb_mregistry_t
{
  fdb_btree_t       m_metadata;          //< btree with the mstruct mapping 
  fdb_pool_alloc_t  m_mstruct_allocator;
  fdb_pool_alloc_t  m_mfield_allocator;
} fdb_mregistry_t;

/**
 * \brief inits a reflection data mregistry
 *
 * \param allocator The allocator to use for the mregistry
 *
 * \return Returns the newly allocated mregistry
 */
void
fdb_mregistry_init(fdb_mregistry_t* reg, 
                   fdb_mem_allocator_t* allocator);

/**
 * \brief Releases the reflection data mregistry
 *
 * \param reg The mregistry to destroy
 */
void
fdb_mregistry_release(fdb_mregistry_t* reg);


/**
 * \brief inits an mstruct mregistry for a given name
 *
 * \param reg The registy to use
 * \param name The name to associate the mstruct with
 * \param is_union Whether is a union or not
 *
 * \return The newly initd mstruct
 */
fdb_mstruct_t*
fdb_mregistry_init_mstruct(fdb_mregistry_t* reg, 
                           const char* name, 
                           bool is_union);

/**
 * \brief inits a mfield in a given mstruct
 *
 * \param reg The mregistry to use
 * \param mstruct The mstruct to init the field to
 * \param name The name of the field
 * \param type The type of the field
 * \param offset The offset in bytes of the field in the mstruct
 * \param is_anon Whether this is an anonymous field or not (e.g. anonymous
 * struct/union)
 *
 * \return If rtype == E_STRUCT or E_UNION-> then a pointer to the mstruct for that struct.
 * Otherwise, nullptr
 */
fdb_mstruct_t*
fdb_mregistry_init_mfield(fdb_mregistry_t* reg, 
                          fdb_mstruct_t* mstruct, 
                          const char* name, 
                          fdb_mtype_t type, 
                          size_t offset, 
                          bool is_anon);

/**
 * \brief Gets the mstruct associated with a given name 
 *
 * \param reg The registry to use
 * \param name The name of the mstruct
 *
 * \return The mstruct or nullptr if it does not exist
 */
fdb_mstruct_t*
fdb_mregistry_get_mstruct(fdb_mregistry_t* reg, 
                          const char* name);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_REFLECTION_H_ */
