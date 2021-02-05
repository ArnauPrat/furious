
#ifndef _FDB_TXBITMAP_H_
#define _FDB_TXBITMAP_H_

#include "../../common/types.h"
#include "../../common/platform.h"
#include "../../common/memory/memory.h"
#include "tx/tx.h"
#include "tx/txpool_allocator.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_TXBITMAP_ALIGNMENT 64
#define FDB_TXBITMAP_NUM_CHUNKS(bits) (bits + 7) / 8


  /**
   * \brief Factory of bitmaps. Bitmaps created through the same factory share 
   * the same underlying memory allocator
   */
  struct fdb_txbitmap_factory_t
  {
    struct fdb_txpool_alloc_t* p_data_alloc;      //< The memory allocator used to store the data blocks of the bitmap
    struct fdb_txpool_alloc_t* p_txbitmap_alloc;  //< The memory allocator used to store the bitmap headers
    uint32_t           m_max_bits;        //< The bitmap size of the bitmaps creted through this factory
  };

  struct fdb_txbitmap_t 
  {
    struct fdb_txbitmap_factory_t*   p_factory;      //< A pointer to the factory this bitmap was created to
    struct fdb_txpool_alloc_ref_t    m_bitmap_ref;   //< Reference to the bitmap header 
  };

  struct fdb_txbitmap_impl_t
  {
    uint32_t m_max_bits;                //< The maximum number of bits
    uint32_t m_num_set;                 //< The number of bits set to one
    struct fdb_txpool_alloc_t* p_txpalloc;      //< The txpool allocator for the bitmap data blocks
    struct fdb_txpool_alloc_ref_t m_data_ref;  //< The buffer with the bitmap
  };


  /**
   * \brief Initializes a bitmap factory. Bitmaps created through a
   * bitmap_factory share allocators and are all of the same size. 
   *
   * \param txbitmap_factory The bitmap factory to initialize.
   * \param allocator The memory allocator to use within the factory. NULL to
   * use the global memory allocator 
   * \param max_bits The maximum number of bits of the bitmaps created by the
   * factory
   */
  void
  fdb_txbitmap_factory_init(struct fdb_txbitmap_factory_t* txbitmap_factory, 
                            struct fdb_mem_allocator_t* allocator, 
                            uint32_t max_bits);

  /**
   * \brief Releases a bitmap factory
   *
   * \param txbitmap_factory The bitmap factory to release
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_factory_release(struct fdb_txbitmap_factory_t* txbitmap_factory, 
                               struct fdb_tx_t* tx, 
                               struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Initializes a bitmap
   *
   * \param bitmap The bitmap to initialize
   * \param bitmap_factory The bitmap factory to use to initilize the bitmap
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_init(struct fdb_txbitmap_t* bitmap,
                    struct fdb_txbitmap_factory_t* bitmap_factory, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief  Releases a bitmap
   *
   * \param bitmap The bitmap to release
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */

  void
  fdb_txbitmap_release(struct fdb_txbitmap_t* bitmap, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Sets to one the ith element of the bitmap
   *
   * \param bitmap The bitmap to perform the operation
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param element The element to set
   */

  void
  fdb_txbitmap_set(struct fdb_txbitmap_t* bitmap, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx,
                   uint32_t element);

  /**
   * \brief Sets to 0 the ith element of the bitmap
   *
   * \param bitmap The bitmap to perform the operation
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param element The ith element to set
   */

  void
  fdb_txbitmap_unset(struct fdb_txbitmap_t* bitmap, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx,
                     uint32_t element);

  /**
   * \brief Sets the bit of the given element to the specified value
   *
   * \param bitmap The bitmap to perform the operation
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param element The element to set the bit for
   * \param value The value to set the bit to
   */

  void
  fdb_txbitmap_set_bit(struct fdb_txbitmap_t* bitmap, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       uint32_t element, 
                       bool value);

  /**
   * \brief Checks if a given element is set to 1
   *
   * \param bitmap The bitmap to perform the operation
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param element The element to check
   *
   * \return True if the element is set to 1
   */

  bool
  fdb_txbitmap_is_set(struct fdb_txbitmap_t* bitmap, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, 
                      uint32_t element);

  /**
   * \brief Sets the bits based on the given bitmap
   *
   * \param dst_bitmap The bitmap to set the bits to 
   * \param src_bitmap The bitmap to set the bits from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */

  void
  fdb_txbitmap_set_txbitmap(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                          FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Ands this bitmap with the given bitmap
   *
   * \param dst_bitmap The bitmap to set the bits to 
   * \param src_bitmap The bitmap to set the bits from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_set_and(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                       FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Ors this bitmap with the given bitmap
   *
   * \param dst_bitmap The bitmap to set the bits to 
   * \param src_bitmap The bitmap to set the bits from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_set_or(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                      FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Diffs this bitmap with the given bitmap
   *
   * \param dst_bitmap The bitmap to set the bits to 
   * \param src_bitmap The bitmap to set the bits from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_set_diff(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                        FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Negates the contents of this bitmap
   *
   * \param bitmap The bitmap to set the bits to 
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_negate(struct fdb_txbitmap_t* bitmap, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Sets the bitmap to all zeros
   *
   * \param bitmap The bitmap to nullify
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txbitmap_nullify(struct fdb_txbitmap_t* bitmap, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_TXBITMAP_H_ */
