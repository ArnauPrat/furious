
#include "furious.h"
#include <gtest/gtest.h>


TEST(TXAllocTest,SimpleTest) 
{
  uint32_t block_size = 128; //bytes
  uint32_t alignment = 8;
  uint32_t page_size = KILOBYTES(4);

  fdb_tx_init(NULL);
  fdb_txpool_alloc_t*  txpool_alloc = fdb_txpool_alloc_create(alignment, 
                                                              block_size, 
                                                              page_size, 
                                                              NULL);


  uint32_t num_allocations = 1024;
  fdb_txpool_alloc_ref_t  refs[num_allocations];
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    refs[i] = {};
  }

  // 1ST
  // Single write transaction
  fdb_tx_t tx_write;
  fdb_tx_begin(&tx_write, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx_write, NULL);


  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    refs[i] = fdb_txpool_alloc_alloc(txpool_alloc, 
                           &tx_write, 
                           txtctx, 
                           alignment, 
                           block_size, 
                           FDB_NO_HINT);

    void* ptr = fdb_txpool_alloc_ptr(txpool_alloc,  
                                     &tx_write, 
                                     txtctx,
                                     refs[i], 
                                     true);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ((uint8_t*)ptr)[j] = i % 256;
    }
    
  }

  fdb_tx_commit(&tx_write); // Commiting write tx. Allocations should be visible
                            // to read tx created from this point
                            
  // 2ND
  // Starting the first read tx. At this point, we should see the references
  fdb_tx_t tx_read;
  fdb_tx_begin(&tx_read, E_READ_ONLY);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(txpool_alloc,  
                                     &tx_read, 
                                     txtctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_read);

  // 3TH
  fdb_tx_begin(&tx_write, E_READ_WRITE);
  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    fdb_txpool_alloc_free(txpool_alloc,  
                          &tx_write, 
                          txtctx,
                          refs[i]);
    
  }

  // 4TH
  // Starting the second read tx. At this point, we should still see the references
  fdb_tx_begin(&tx_read, E_READ_ONLY);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(txpool_alloc,  
                                     &tx_read, 
                                     txtctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_write);

  for(uint32_t i = 0; i < num_allocations; ++i)
  {
    void* ptr = fdb_txpool_alloc_ptr(txpool_alloc,  
                                     &tx_read, 
                                     txtctx,
                                     refs[i], 
                                     false);
    ASSERT_NE((uint64_t)ptr, NULL);

    for(uint32_t j = 0; j < block_size; ++j)
    {
      ASSERT_EQ(((uint8_t*)ptr)[j], i % 256);
    }
  }
  fdb_tx_commit(&tx_read);

  fdb_tx_begin(&tx_write, E_READ_WRITE);
  fdb_txpool_alloc_destroy(txpool_alloc,
                           &tx_write, 
                           txtctx);
  fdb_tx_commit(&tx_write);
  fdb_tx_release();
}

typedef struct thread_data_t
{
  fdb_tx_t                  m_tx;
  fdb_txtype_t              m_txtype;
  fdb_txpool_alloc_t*       p_alloc;
  fdb_txpool_alloc_ref_t*   m_refs;
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
      struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tdata->m_tx, NULL);
      for(uint32_t i = 0; i < tdata->m_nrefs; ++i)
      {
        void* ptr = fdb_txpool_alloc_ptr(tdata->p_alloc, 
                                         &tdata->m_tx, 
                                         txtctx, 
                                         tdata->m_refs[i], 
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
      struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tdata->m_tx, NULL);
      for(uint32_t i = 0; i < tdata->m_nrefs; ++i)
      {
        void* ptr = fdb_txpool_alloc_ptr(tdata->p_alloc, 
                                         &tdata->m_tx, 
                                         txtctx,
                                         tdata->m_refs[i], 
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
  fdb_tx_init(NULL);

  uint32_t niters = 10;
  uint64_t nelems = 8;
  uint32_t alignment = 8;
  uint32_t block_size = sizeof(uint64_t)*nelems;
  uint32_t page_size = KILOBYTES(4);
  fdb_txpool_alloc_t* txpool_alloc = fdb_txpool_alloc_create(alignment, 
                                                             block_size, 
                                                             page_size, 
                                                             NULL);

  uint32_t num_refs = 8;
  fdb_txpool_alloc_ref_t* refs = new fdb_txpool_alloc_ref_t[num_refs];
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    refs[i] = {};
  }



  fdb_tx_t tx_init;
  fdb_tx_begin(&tx_init, E_READ_WRITE);
  fdb_txthread_ctx_t* context_init = fdb_tx_txthread_ctx_get(&tx_init, NULL);
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    refs[i] = fdb_txpool_alloc_alloc(txpool_alloc, 
                                     &tx_init, 
                                     context_init, 
                                     alignment, 
                                     block_size, 
                                     FDB_NO_HINT);

    uint64_t* elements = (uint64_t*) fdb_txpool_alloc_ptr(txpool_alloc, 
                                                          &tx_init, 
                                                          context_init, 
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
    data->m_refs = refs;
    data->m_nrefs = num_refs;
    data->m_txtype = E_READ_ONLY;
    data->m_niters = niters;
    data->m_alignment = alignment;
  }

  
  uint32_t num_writers = 16;
  thread_data_t writer_data[num_writers];
  for(uint32_t i = 0; i < num_writers; ++i)
  {
    thread_data_t* data = &writer_data[i];
    data->p_alloc = txpool_alloc;
    data->m_nelems = nelems;
    data->m_refs = refs;
    data->m_nrefs = num_refs;
    data->m_txtype = E_READ_WRITE;
    data->m_niters = niters;
    data->m_alignment = alignment;
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
  }

  for(uint32_t i = 0; i < num_writers; ++i)
  {
    fdb_thread_join(&writer_threads[i]);
  }

  
  // Testing that all old data have been properly freed
  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);
  context_init = fdb_tx_txthread_ctx_get(&tx_init, NULL);

  // This access should eliminate versions and create zombies
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    fdb_txpool_alloc_ptr(txpool_alloc, 
                         &tx_init, 
                         context_init, 
                         refs[i], 
                         false);
  }
  fdb_tx_commit(&tx_init);

  // We need to run a writting transaction in order to be able to remove zombies
  // safely, since there is a latency of one transaction between zombie creation
  // and removal
  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);
  fdb_tx_commit(&tx_init);


  // This should safely eliminate zombies
  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);
  context_init = fdb_tx_txthread_ctx_get(&tx_init, NULL);
  for(uint32_t i = 0; i < num_refs; ++i)
  {
    fdb_txpool_alloc_ptr(txpool_alloc, 
                         &tx_init, 
                         context_init, 
                         refs[i], 
                         false);

    fdb_txpool_alloc_free(txpool_alloc, 
                          &tx_init, 
                          context_init, 
                          refs[i]);
  }
  fdb_tx_commit(&tx_init);

  fdb_tx_begin(&tx_init, 
               E_READ_WRITE);
  fdb_txpool_alloc_destroy(txpool_alloc, 
                           &tx_init,
                           context_init);
  fdb_tx_commit(&tx_init);

  delete [] refs;
  fdb_tx_release();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

