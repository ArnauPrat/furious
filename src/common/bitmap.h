
#ifndef _FURIOUS_BITMAP_H_
#define _FURIOUS_BITMAP_H_ value

#include "types.h"
#include "platform.h"
#include "memory/memory.h"

namespace furious
{

#define FURIOUS_BITMAP_ALIGNMENT 64

struct bitmap_t 
{
  uint32_t m_max_bits;          //< The maximum number of bits
  uint32_t m_num_set;           //< The number of bits set to one
  FURIOUS_ALIGNED(uint8_t*,p_data,FURIOUS_BITMAP_ALIGNMENT);  //< The buffer with the bitmap
};

/**
 * \brief Initializes a bitmap
 *
 * \return  The created bitmap
 */
bitmap_t
bitmap_create(uint32_t max_bits, mem_allocator_t* allocator);

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */

void
bitmap_destroy(bitmap_t* bitmap, mem_allocator_t* allocator);

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */

void
bitmap_set(bitmap_t* bitmap, 
           uint32_t element);

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */

void
bitmap_unset(bitmap_t* bitmap, 
             uint32_t element);

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */

void
bitmap_set_bit(bitmap_t* bitmap, 
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
bitmap_is_set(const bitmap_t* bitmap, 
              uint32_t element);

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
bitmap_set_bitmap(FURIOUS_RESTRICT(bitmap_t*) dst_bitmap, 
                  FURIOUS_RESTRICT(const bitmap_t*) src_bitmap);

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
bitmap_set_and(bitmap_t* dst_bitmap, 
               const bitmap_t* src_bitmap);

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
bitmap_set_or(bitmap_t* dst_bitmap, 
              const bitmap_t* src_bitmap);

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */

void
bitmap_set_diff(bitmap_t* dst_bitmap, 
                const bitmap_t* src_bitmap);

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */

void
bitmap_negate(bitmap_t* bitmap);

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
bitmap_nullify(bitmap_t* bitmap);

} /* furious
 */ 
#endif /* ifndef _FURIOUS_BITMAP_H_ */
