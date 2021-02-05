
#include "../../runtime/data/txbitmap.h"
#include "../../runtime/data/txbitmap_utils.h"

#include <gtest/gtest.h>
#include <set>

TEST(BitmapTest, BitmapTest ) 
{
  constexpr uint32_t BITMAP_SIZE = 304;

  fdb_tx_init(NULL);

  fdb_txbitmap_factory_t bitmap_factory;
  fdb_txbitmap_factory_init(&bitmap_factory, NULL, BITMAP_SIZE);

  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbitmap_t bitmap;
  fdb_txbitmap_init(&bitmap, 
                    &bitmap_factory, 
                    &tx, 
                    txtctx);

  fdb_txbitmap_impl_t* bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                                                &tx, 
                                                                                txtctx,
                                                                                bitmap.m_bitmap_ref, 
                                                                                false);


  ASSERT_EQ(bitmap_impl->m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                  &tx, 
                                  txtctx, 
                                  i), false);
    fdb_txbitmap_set(&bitmap,
                   &tx, 
                   txtctx, 
                   i);
  }

  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);
  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    ASSERT_EQ(fdb_txbitmap_is_set(&bitmap, 
                                  &tx,
                                  txtctx, 
                                  i), true);
    fdb_txbitmap_unset(&bitmap,
                     &tx, 
                     txtctx, 
                     i);
    ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                  &tx, 
                                  txtctx, 
                                  i), false);
  }

  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);
  ASSERT_EQ(bitmap_impl->m_num_set, 0);

  for(uint32_t i = 0; i < BITMAP_SIZE; i+=2)
  {
    ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                &tx, 
                                txtctx,
                                i), false);
    fdb_txbitmap_set(&bitmap,
                     &tx, 
                     txtctx, 
                     i);
    ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                  &tx, 
                                  txtctx, 
                                  i), true);
  }

  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);

  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap, 
                                  &tx, 
                                  txtctx, 
                                  i), true);
      fdb_txbitmap_unset(&bitmap,
                         &tx, 
                         txtctx, 
                         i);
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                  &tx, 
                                  txtctx, 
                                  i), false);
    }
    else
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), false);
      fdb_txbitmap_set(&bitmap,
                       &tx, 
                       txtctx, 
                       i);

      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), true);
    }
  }

  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);

  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE/2);

  fdb_txbitmap_negate(&bitmap, 
                      &tx, 
                      txtctx);

  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);

  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), true);
      fdb_txbitmap_unset(&bitmap,
                         &tx, 
                         txtctx, 
                         i);
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                  &tx, 
                                  txtctx, 
                                  i), false);
    }
    else
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), false);
      fdb_txbitmap_set(&bitmap,
                     &tx, 
                     txtctx, 
                     i);
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), true);
    }
  }
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);
  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE/2);

  fdb_txbitmap_t bitmap2;
  fdb_txbitmap_init(&bitmap2, 
                    &bitmap_factory,
                    &tx, 
                    txtctx);
  fdb_txbitmap_set_txbitmap(&bitmap2,
                          &bitmap, 
                          &tx, 
                          txtctx);

  fdb_txbitmap_negate(&bitmap2, 
                    &tx, 
                    txtctx);

  fdb_txbitmap_set_or(&bitmap,
                      &bitmap2, 
                      &tx, 
                      txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);
  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE);
  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), true);
  }

  fdb_txbitmap_set_and(&bitmap,
                       &bitmap2, 
                       &tx, 
                       txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);

  bitmap_impl = (fdb_txbitmap_impl_t*)fdb_txpool_alloc_ptr(bitmap.p_factory->p_txbitmap_alloc, 
                                                           &tx, 
                                                           txtctx,
                                                           bitmap.m_bitmap_ref, 
                                                           false);
  ASSERT_EQ(bitmap_impl->m_num_set, BITMAP_SIZE/2);

  for(uint32_t i = 0; i < BITMAP_SIZE; ++i)
  {
    if(i % 2 == 0)
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), true);
    }
    else
    {
      ASSERT_EQ(fdb_txbitmap_is_set(&bitmap,
                                    &tx, 
                                    txtctx, 
                                    i), false);
    }
  }
  fdb_txbitmap_release(&bitmap,
                     &tx, 
                     txtctx);

  fdb_txbitmap_release(&bitmap2,
                       &tx,
                       txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txbitmap_factory_release(&bitmap_factory, 
                               &tx, 
                               txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

