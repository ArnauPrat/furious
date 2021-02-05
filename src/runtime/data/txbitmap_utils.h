
#ifndef _FDB_TXBITMAP_UTILS_H_
#define _FDB_TXBITMAP_UTILS_H_

#include "txbitmap.h"
#include "../../common/bitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_txbitmap(FDB_RESTRICT(struct fdb_bitmap_t*) dst_bitmap, 
                        FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_bitmap(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                        FDB_RESTRICT(struct fdb_bitmap_t*) src_bitmap,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_txbitmap_and(struct fdb_bitmap_t* dst_bitmap, 
                            struct fdb_txbitmap_t* src_bitmap,
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_bitmap_and(struct fdb_txbitmap_t* dst_bitmap, 
                            struct fdb_bitmap_t* src_bitmap,
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_txbitmap_or(struct fdb_bitmap_t* dst_bitmap, 
                           struct fdb_txbitmap_t* src_bitmap,
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_bitmap_or(struct fdb_txbitmap_t* dst_bitmap, 
                           struct fdb_bitmap_t* src_bitmap,
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_txbitmap_diff(struct fdb_bitmap_t* dst_bitmap, 
                             struct fdb_txbitmap_t* src_bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_bitmap_diff(struct fdb_txbitmap_t* dst_bitmap, 
                             struct fdb_bitmap_t* src_bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx);

#ifdef __cplusplus
}
#endif

#endif 
