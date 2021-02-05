
#ifndef _FDB_BITMAP_H_
#define _FDB_BITMAP_H_

#include "types.h"
#include "platform.h"
#include "memory/memory.h"
#include "memory/pool_allocator.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_BITMAP_ALIGNMENT 64
#define FDB_BITMAP_NUM_CHUNKS(bits) (bits + 7) / 8

struct fdb_bitmap_factory_t
{
  struct fdb_pool_alloc_t    m_allocator;   //< The memory allocation used to allocate bitmap data
  uint32_t                   m_data_size;   //< The number of bytes to store bitmap data;
  uint32_t                   m_max_bits;    //< The maximum number of bits of the bitmaps created with this factory
};

struct fdb_bitmap_t 
{
  struct fdb_bitmap_factory_t*       p_factory; //< Pointer to the factory this bitmap belongs to
  uint32_t m_num_set;           //< The number of bits set to one
  FDB_ALIGNED(uint8_t*,p_data,FDB_BITMAP_ALIGNMENT);  //< The buffer with the bitmap
};

/**
 * \brief Initializes a bitmap factory
 *
 * \param factory The factory to initialize
 * \param max_bits The maximum number of bits of the factory
 * \param allocator The memory allocator to use for this factory
 */
void
fdb_bitmap_factory_init(struct fdb_bitmap_factory_t* factory, 
                   uint32_t max_bits, 
                   struct fdb_mem_allocator_t* allocator);

void
/**
 * \brief Releases a bitmap factory
 *
 * \param factory The bitmap factory to release
 */
fdb_bitmap_factory_release(struct fdb_bitmap_factory_t* factory);

/**
 * \brief Initializes a bitmap
 *
 * \return  The initd bitmap
 */
void
fdb_bitmap_init(struct fdb_bitmap_t* bitmap,
                struct fdb_bitmap_factory_t* bitmap_factory);

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap The bitmap to release
 *
 */

void
fdb_bitmap_release(struct fdb_bitmap_t* bitmap);

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */

void
fdb_bitmap_set(struct fdb_bitmap_t* bitmap, 
               uint32_t element);

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */
void
fdb_bitmap_unset(struct fdb_bitmap_t* bitmap, 
                 uint32_t element);

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */
void
fdb_bitmap_set_bit(struct fdb_bitmap_t* bitmap, 
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
fdb_bitmap_is_set(const struct fdb_bitmap_t* bitmap, 
                  uint32_t element);

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_bitmap(FDB_RESTRICT(struct fdb_bitmap_t*) dst_bitmap, 
                      FDB_RESTRICT(const struct fdb_bitmap_t*) src_bitmap);

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_and(struct fdb_bitmap_t* dst_bitmap, 
                   const struct fdb_bitmap_t* src_bitmap);

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_or(struct fdb_bitmap_t* dst_bitmap, 
                  const struct fdb_bitmap_t* src_bitmap);

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_diff(struct fdb_bitmap_t* dst_bitmap, 
                    const struct fdb_bitmap_t* src_bitmap);

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */
void
fdb_bitmap_negate(struct fdb_bitmap_t* bitmap);

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
fdb_bitmap_nullify(struct fdb_bitmap_t* bitmap);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_BITMAP_H_ */
