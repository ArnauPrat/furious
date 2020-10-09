
#include "furious.h"
#include <gtest/gtest.h>

/*
TEST(TXAllocTest,SimpleTest) 
{
  uint32_t block_size = 128; //bytes
  uint32_t alignment = 8;
  uint32_t page_size = KILOBYTES(4);

  fdb_tx_init();
  fdb_txpool_alloc_t txpool_alloc;
  fdb_txpool_alloc_init(&txpool_alloc, 
                        alignment, 
                        block_size, 
                        page_size, 
                        NULL);

  fdb_txthread_ctx_t thread_ctx;
  fdb_txthread_ctx_init(&thread_ctx, 
                        NULL);

  // 1ST
  // Single write transaction
  fdb_tx_t tx_write;
  fdb_tx_begin(&tx_write, E_READ_WRITE);


  uint32_t num_allocations = 1024;
  fdb_txpool_alloc_ref_t* refs[num_allocations];
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    refs[i] = fdb_txpool_alloc_alloc(&txpool_alloc, 
                                     &tx_write, 
                                     &thread_ctx, 
                                     alignment, 
                                     block_size, 
                                     FDB_NO_HINT);

    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_write, 
                                     &thread_ctx,
                                     refs[i], 
                                     true);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ((uint8_t*)ptr)[j] = i % 256;
    }
    
  }

  // 2ND
  fdb_tx_t tx_read;
  fdb_tx_begin(&tx_read, E_READ_ONLY);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_read, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_EQ((uint64_t)ptr, NULL);
  }

  fdb_tx_commit(&tx_write); // Commiting write tx. Allocations should be visible
                            // to read tx created from this point
                            
  // Checking that the read transaction still gets NULL from references
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_read, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_EQ((uint64_t)ptr, NULL);
  }

  // Committing the first read tx
  fdb_tx_commit(&tx_read);

  // 3RD
  // Starting the second read tx. At this point, we should see the references
  fdb_tx_begin(&tx_read, E_READ_ONLY);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_read, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_read);

  // 4TH
  fdb_tx_begin(&tx_write, E_READ_WRITE);
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    fdb_txpool_alloc_free(&txpool_alloc,  
                          &tx_write, 
                          &thread_ctx,
                          refs[i]);
    
  }

  // Reading again the reference from the  write transaction should return NULL,
  // given that they have already been freed by this transaction
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_write, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_EQ((uint64_t)ptr, NULL);
  }

  // 5TH
  // Starting the third read tx. At this point, we should still see the references
  fdb_tx_begin(&tx_read, E_READ_ONLY);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_read, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_write);

  // Call to gc should not GC the freed blocks because there is still a read
  // transaction running on that freed versions
  ASSERT_FALSE(fdb_txthread_ctx_gc(&thread_ctx));

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(&txpool_alloc,  
                                     &tx_read, 
                                     &thread_ctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_read);

  // At this point, freed blocks should be garbage collected
  ASSERT_TRUE(fdb_txthread_ctx_gc(&thread_ctx));
  fdb_txthread_ctx_release(&thread_ctx);

  fdb_txpool_alloc_release(&txpool_alloc);
  fdb_tx_release();
}
*/

typedef struct thread_data_t
{
  fdb_txthread_ctx_t        m_context;
  fdb_tx_t                  m_tx;
  fdb_txtype_t              m_txtype;
  fdb_txpool_alloc_t        p_alloc;
  fdb_txpool_alloc_ref_t**  p_refs;
  uint32_t                  m_nrefs;
  uint32_t                  m_nelems;
  uint32_t                  m_niters;;
  uint32_t                  m_alignment;
} thread_data_t;

void thread_func(void* arg)
{
  thread_data_t* tdata = (thread_data_t*)arg;
  if(tdata->m_txtype == E_READ_ONLY)
  {
    for(uint32_t k = 0; k < tdata->m_niters; ++k)
    {
      // Read transaction
      fdb_tx_begin(&tdata->m_tx, tdata->m_txtype);
      for(uint32_t i = 0; i < tdata->m_nrefs; ++i)
      {
        void* ptr = fdb_txpool_alloc_ptr(&tdata->p_alloc, 
                                         &tdata->m_tx, 
                                         &tdata->m_context, 
                                         tdata->p_refs[i], 
                                         false);
        ASSERT_TRUE((uint64_t)ptr % tdata->m_alignment == 0);
        uint64_t* elements = (uint64_t*) ptr;
        for(uint32_t j = 0; j < tdata->m_nelems; ++j)
        {
          ASSERT_EQ(elements[j], tdata->m_tx.m_txversion);
        }
      }
      fdb_tx_commit(&tdata->m_tx);
    }
  }
  else
  {
    // Read/Write transaction
    for(uint32_t k = 0; k < tdata->m_niters; ++k)
    {
      // Read/Write transaction
      fdb_tx_begin(&tdata->m_tx, tdata->m_txtype);
      for(uint32_t i = 0; i < tdata->m_nrefs; ++i)
      {
        void* ptr = fdb_txpool_alloc_ptr(&tdata->p_alloc, 
                                         &tdata->m_tx, 
                                         &tdata->m_context, 
                                         tdata->p_refs[i], 
                                         true);
        ASSERT_TRUE((uint64_t)ptr % tdata->m_alignment == 0);

        uint64_t* elements = (uint64_t*) ptr;
        for(uint32_t j = 0; j < tdata->m_nelems; ++j)
        {
          elements[j] = tdata->m_tx.m_txversion;
        }
      }
      fdb_tx_commit(&tdata->m_tx);
    }
  }
}

TEST(TXAllocTest,ConcurrentMultipleReadMultipleWrite) 
{
  fdb_tx_init();

  uint32_t niters = 10;
  uint64_t nelems = 8;
  uint32_t alignment = 8;
  uint32_t block_size = sizeof(uint64_t)*nelems;
  uint32_t page_size = KILOBYTES(4);
  fdb_txpool_alloc_t txpool_alloc;
  fdb_txpool_alloc_init(&txpool_alloc, 
                        alignment, 
                        block_size, 
                        page_size, 
                        NULL);

  uint32_t num_refs = 8;
  fdb_txpool_alloc_ref_t** refs = new fdb_txpool_alloc_ref_t*[num_refs];



  fdb_txthread_ctx_t context_init;
  fdb_txthread_ctx_init(&context_init, NULL);
  fdb_tx_t tx_init;
  fdb_tx_begin(&tx_init, E_READ_WRITE);
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    refs[i]= fdb_txpool_alloc_alloc(&txpool_alloc, 
                                    &tx_init, 
                                    &context_init, 
                                    alignment, 
                                    block_size, 
                                    FDB_NO_HINT);

    uint64_t* elements = (uint64_t*) fdb_txpool_alloc_ptr(&txpool_alloc, 
                                                          &tx_init, 
                                                          &context_init, 
                                                          refs[i], 
                                                          true);
    for(uint32_t j = 0; j < nelems; ++j)
    {
      elements[j] = tx_init.m_txversion;
    }
  }
  fdb_tx_commit(&tx_init);


  uint32_t num_readers = 2;
  thread_data_t reader_data[num_readers];
  for(uint32_t i = 0; i < num_readers; ++i)
  {
    thread_data_t* data = &reader_data[i];
    data->p_alloc = txpool_alloc;
    data->m_nelems = nelems;
    data->p_refs = refs;
    data->m_nrefs = num_refs;
    data->m_txtype = E_READ_ONLY;
    data->m_niters = niters;
    data->m_alignment = alignment;
    fdb_txthread_ctx_init(&data->m_context, 
                          NULL);
  }

  
  uint32_t num_writers = 4;
  thread_data_t writer_data[num_writers];
  for(uint32_t i = 0; i < num_writers; ++i)
  {
    thread_data_t* data = &writer_data[i];
    data->p_alloc = txpool_alloc;
    data->m_nelems = nelems;
    data->p_refs = refs;
    data->m_nrefs = num_refs;
    data->m_txtype = E_READ_WRITE;
    data->m_niters = niters;
    data->m_alignment = alignment;
    fdb_txthread_ctx_init(&data->m_context, 
                          NULL);
  }

  fdb_thread_task_t reader_tasks[num_readers];
  fdb_thread_t reader_threads[num_readers];
  for(uint32_t i = 0; i < num_readers; ++i)
  {
    fdb_thread_task_t* task = &reader_tasks[i];
    task->p_fp = thread_func;
    task->p_args = &reader_data[i];
    fdb_thread_start(&reader_threads[i], task);
  }

  fdb_thread_task_t writer_tasks[num_writers];
  fdb_thread_t writer_threads[num_writers];
  for(uint32_t i = 0; i < num_writers; ++i)
  {
    fdb_thread_task_t* task = &writer_tasks[i];
    task->p_fp = thread_func;
    task->p_args = &writer_data[i];
    fdb_thread_start(&writer_threads[i], task);
  }

  for(uint32_t i = 0; i < num_readers; ++i)
  {
    fdb_thread_join(&reader_threads[i]);
    while(!fdb_txthread_ctx_gc(&reader_data[i].m_context))
    {
    }

    fdb_txthread_ctx_release(&reader_data[i].m_context);
  }

  for(uint32_t i = 0; i < num_writers; ++i)
  {
    fdb_thread_join(&writer_threads[i]);
    while(!fdb_txthread_ctx_gc(&writer_data[i].m_context))
    {
    }

    fdb_txthread_ctx_release(&writer_data[i].m_context);
  }

  /*
  // Testing that all old data have been properly freed
  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);

  // This access should eliminate versions and create zombies
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    fdb_txpool_alloc_ptr(&txpool_alloc, 
                         &tx_init, 
                         &context_init, 
                         refs[i], 
                         false);
    ASSERT_EQ((uint64_t)refs[i]->p_next_ref, NULL);
  }
  fdb_tx_commit(&tx_init);


  // This should safely eliminate zombies
  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    fdb_txpool_alloc_ptr(&txpool_alloc, 
                         &tx_init, 
                         &context_init, 
                         refs[i], 
                         false);
    ASSERT_EQ((uint64_t)refs[i]->p_next_ref, NULL);
    ASSERT_EQ((uint64_t)refs[i]->p_zombie, NULL);

    fdb_txpool_alloc_free(&txpool_alloc, 
                          &tx_init, 
                          &context_init, 
                          refs[i]);
  }
  fdb_tx_commit(&tx_init);

  */
  ASSERT_TRUE(fdb_txthread_ctx_gc(&context_init));
  fdb_txthread_ctx_release(&context_init);

  delete [] refs;
  fdb_txpool_alloc_release(&txpool_alloc);
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

