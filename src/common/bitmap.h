
#ifndef _FDB_BITMAP_H_
#define _FDB_BITMAP_H_

#include "types.h"
#include "platform.h"
#include "memory/memory.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_BITMAP_ALIGNMENT 64
#define FDB_BITMAP_NUM_CHUNKS(bits) (bits + 7) / 8

typedef struct fdb_bitmap_t 
{
  uint32_t m_max_bits;          //< The maximum number of bits
  uint32_t m_num_set;           //< The number of bits set to one
  FDB_ALIGNED(uint8_t*,p_data,FDB_BITMAP_ALIGNMENT);  //< The buffer with the bitmap
} fdb_bitmap_t;

/**
 * \brief Initializes a bitmap
 *
 * \return  The initd bitmap
 */

void
fdb_bitmap_init(fdb_bitmap_t* bitmap,
                uint32_t max_bits, 
                fdb_mem_allocator_t* allocator);

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */

void
fdb_bitmap_release(fdb_bitmap_t* bitmap, fdb_mem_allocator_t* allocator);

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */

void
fdb_bitmap_set(fdb_bitmap_t* bitmap, 
               uint32_t element);

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */

void
fdb_bitmap_unset(fdb_bitmap_t* bitmap, 
                 uint32_t element);

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */

void
fdb_bitmap_set_bit(fdb_bitmap_t* bitmap, 
                   uint32_t element, 
                   bool value);

/**
 * \brief Checks if a given element is set to 1
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to check
 *
 * \return True if the element is set to 1
 */

bool
fdb_bitmap_is_set(const fdb_bitmap_t* bitmap, 
                  uint32_t element);

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
fdb_bitmap_set_bitmap(FDB_RESTRICT(fdb_bitmap_t*) dst_bitmap, 
                      FDB_RESTRICT(const fdb_bitmap_t*) src_bitmap);

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
fdb_bitmap_set_and(fdb_bitmap_t* dst_bitmap, 
                   const fdb_bitmap_t* src_bitmap);

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
fdb_bitmap_set_or(fdb_bitmap_t* dst_bitmap, 
                  const fdb_bitmap_t* src_bitmap);

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
fdb_bitmap_set_diff(fdb_bitmap_t* dst_bitmap, 
                    const fdb_bitmap_t* src_bitmap);

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */

void
fdb_bitmap_negate(fdb_bitmap_t* bitmap);

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
fdb_bitmap_nullify(fdb_bitmap_t* bitmap);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_BITMAP_H_ */
