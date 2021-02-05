

#ifndef _FURIOUS_REF_TABLE_H_
#define _FURIOUS_REF_TABLE_H_

#include "../../common/types.h"
#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_reftable_t
{
  fdb_table_t m_table;
} fdb_reftable_t;


/**
 * \brief inits a reference table
 *
 * \param The name of the reference
 * \param allocator The allocator to use
 *
 * \return The newly initd reference table
 */
void
fdb_reftable_init(fdb_reftable_t* rt, 
                  const char* rname, 
                  uint32_t id, 
                  fdb_mem_allocator_t* allocator);

/**
 * \brief releases a reference table
 *
 * \param rt The reference table to release
 */
void
fdb_reftable_release(fdb_reftable_t* rt);

/**
 * \brief Adds a reference to reftable
 *
 * \param rt The reference table 
 * \param tail The tail of the reference
 * \param head The head of the reference
 */
void
fdb_reftable_add(fdb_reftable_t* rt,
                 entity_id_t tail, 
                 entity_id_t head);

/**
 * \brief Gets the reference for the given tail
 *
 * \param rt The reference table 
 * \param tail The tail
 *
 * \return Returns a pointer to the reference or  false if it does not exist
 */
entity_id_t*
fdb_reftable_get(fdb_reftable_t* rt, 
                 entity_id_t tail);

/**
 * \brief Removes a reference from the ref table
 *
 * \param rt The reference table 
 * \param tail The tail of the reference
 * \param head The head of the reference
 */
void
fdb_reftable_remove(fdb_reftable_t* rt,
                    entity_id_t tail, 
                    entity_id_t head);

/**
 * \brief Checks if a reference exists
 *
 * \param rt The reference table 
 * \param tail The tail of the reference
 * \param head The head of the reference
 */
bool
fdb_reftable_exists(fdb_reftable_t* rt, 
                    entity_id_t tail, 
                    entity_id_t head);
#ifdef __cplusplus
}
#endif

#endif 
