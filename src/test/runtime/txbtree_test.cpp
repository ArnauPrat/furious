
#include "../../common/platform.h"
#include "../../runtime/data/txbtree.h"
#include "../../common/hashtable.h"

#include <gtest/gtest.h>

//extern int32_t fdb_btree_num_allocations;

struct TestValue 
{
  uint32_t m_val;
};

TEST(txbtree_test, fdb_btree_create) 
{
  // Initialize thread context
  fdb_tx_init(NULL);
  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);


  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);


  fdb_txpool_alloc_ref_t internal_ref = fdb_txbtree_create_internal(&btree, 
                                                                    &tx, 
                                                                    txtctx);

  fdb_txbtree_node_t* internal = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                           &tx, 
                                                                           txtctx, 
                                                                           internal_ref, 
                                                                           false);
  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_TRUE(internal->m_internal.m_children[i].p_main == NULL);
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_EQ(internal->m_internal.m_keys[i], 0);
  }

  ASSERT_EQ(internal->m_internal.m_nchildren, 0);
  fdb_txbtree_destroy_node(&btree, 
                           &tx, 
                           txtctx, 
                           internal_ref);

  fdb_txpool_alloc_ref_t leaf_ref =  fdb_txbtree_create_leaf(&btree, 
                                                             &tx, 
                                                             txtctx);

  fdb_txbtree_node_t* leaf = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       leaf_ref, 
                                                                       false);

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(leaf->m_leaf.m_leafs[i].p_main, nullptr);
    ASSERT_EQ(leaf->m_leaf.m_keys[i], 0);
  }

  ASSERT_EQ(leaf->m_leaf.m_nleafs, 0);
  ASSERT_TRUE(leaf->m_leaf.m_next.p_main == NULL);
  fdb_txbtree_destroy_node(&btree, &tx, txtctx, leaf_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, &tx, txtctx );
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

TEST(txbtree_test, fdb_btree_next_internal) 
{

  fdb_tx_init(NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);


  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);



  fdb_txpool_alloc_ref_t internal_ref = fdb_txbtree_create_internal(&btree, 
                                                                    &tx, 
                                                                    txtctx);

  fdb_txbtree_node_t* internal = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                           &tx, 
                                                                           txtctx, 
                                                                           internal_ref, 
                                                                           true);

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    internal->m_internal.m_children[i] = fdb_txbtree_create_internal(&btree, 
                                                                     &tx, 
                                                                     txtctx);
    internal->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    internal->m_internal.m_keys[i-1] = i*10;
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_txbtree_next_internal(&btree, &tx, txtctx, internal_ref,i*10), i);
  }

  fdb_txbtree_destroy_node(&btree, &tx, txtctx, internal_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(txbtree_test, fdb_btree_next_leaf) 
{
  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);


  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);


  fdb_txpool_alloc_ref_t leaf_ref = fdb_txbtree_create_leaf(&btree, &tx, txtctx);
  fdb_txbtree_node_t* leaf = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       leaf_ref,
                                                                       true);

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    fdb_txpool_alloc_ref_t value_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                                                              &tx, 
                                                              txtctx, 
                                                              FDB_MIN_ALIGNMENT, 
                                                              sizeof(TestValue),
                                                              FDB_NO_HINT);

    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx, 
                                                         txtctx, 
                                                         value_ref, 
                                                         true);
    value->m_val = i*2;
    leaf->m_leaf.m_leafs[i] = value_ref;
    leaf->m_leaf.m_keys[i] = i*2;
    leaf->m_leaf.m_nleafs++;
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_txbtree_next_leaf(&btree, &tx, txtctx, leaf_ref,i*2), i);
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_txbtree_next_leaf(&btree, &tx, txtctx,leaf_ref,i*2+1), i+1);
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    fdb_txpool_alloc_free(pvalsalloc, 
                          &tx, 
                          txtctx, 
                          leaf->m_leaf.m_leafs[i]);
  }

  fdb_txbtree_destroy_node(&btree, &tx, txtctx, leaf_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(txbtree_test, fdb_btree_split_internal) 
{

  fdb_tx_init(NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);


  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_internal(&btree, &tx, txtctx);

  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       node_ref, 
                                                                       true);

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = fdb_txbtree_create_leaf(&btree, &tx, txtctx);
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_keys[i-1] = i*10;
  }

  uint32_t sibling_key;
  fdb_txpool_alloc_ref_t sibling_ref =  fdb_txbtree_split_internal(&btree, 
                                                                   &tx, 
                                                                   txtctx, 
                                                                   node_ref, 
                                                                   &sibling_key);

  fdb_txbtree_node_t* sibling = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                          &tx, 
                                                                          txtctx, 
                                                                          sibling_ref, 
                                                                          false);
  ASSERT_NE(node->m_internal.m_nchildren, 0);
  ASSERT_NE(sibling->m_internal.m_nchildren, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(node->m_internal.m_children[i].p_main != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(sibling->m_internal.m_children[i].p_main != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, FDB_TXBTREE_INTERNAL_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_internal.m_nchildren + sibling->m_internal.m_nchildren);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < FDB_TXBTREE_INTERNAL_MAX_ARITY -1 && node->m_internal.m_children[i+1].p_main != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_internal.m_keys[i] > node->m_internal.m_keys[i-1]);
    max_key = node->m_internal.m_keys[i];
  }

  for (uint32_t i = 1; i < FDB_TXBTREE_INTERNAL_MAX_ARITY -1 && sibling->m_internal.m_children[i+1].p_main != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > sibling->m_internal.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > max_key);
  }

  fdb_txbtree_destroy_node(&btree, 
                           &tx, 
                           txtctx,
                           sibling_ref);
  fdb_txbtree_destroy_node(&btree, 
                           &tx, 
                           txtctx,
                           node_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

TEST(txbtree_test, fdb_btree_split_leaf) 
{

  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);


  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_leaf(&btree, 
                                                            &tx, 
                                                            txtctx);

  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       node_ref, 
                                                                       true);

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    fdb_txpool_alloc_ref_t value_ref = fdb_txpool_alloc_alloc(pvalsalloc,
                                                              &tx, 
                                                              txtctx, 
                                                              FDB_MIN_ALIGNMENT, 
                                                              sizeof(TestValue),
                                                              FDB_NO_HINT);

    TestValue* value = (TestValue*)fdb_txpool_alloc_ptr(pvalsalloc, 
                                                        &tx, 
                                                        txtctx, 
                                                        value_ref, 
                                                        true);
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value_ref;
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  uint32_t sibling_key;
  fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_split_leaf(&btree, 
                                                              &tx, 
                                                              txtctx, 
                                                              node_ref, 
                                                              &sibling_key);
  fdb_txbtree_node_t* sibling = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                          &tx, 
                                                                          txtctx, 
                                                                          sibling_ref, 
                                                                          false);

  ASSERT_NE(node->m_leaf.m_nleafs, 0);
  ASSERT_NE(sibling->m_leaf.m_nleafs, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(node->m_leaf.m_leafs[i].p_main != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(sibling->m_leaf.m_leafs[i].p_main != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, FDB_TXBTREE_LEAF_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_leaf.m_nleafs + sibling->m_leaf.m_nleafs);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < FDB_TXBTREE_LEAF_MAX_ARITY && node->m_leaf.m_leafs[i].p_main != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_leaf.m_keys[i] > node->m_leaf.m_keys[i-1]);
    max_key = node->m_leaf.m_keys[i];
    fdb_txpool_alloc_free(pvalsalloc, 
                          &tx, 
                          txtctx, 
                          node->m_leaf.m_leafs[i]);
  }
  fdb_txpool_alloc_free(pvalsalloc, 
                        &tx, 
                        txtctx, 
                        node->m_leaf.m_leafs[0]);

  for (uint32_t i = 1; i < FDB_TXBTREE_LEAF_MAX_ARITY && sibling->m_leaf.m_leafs[i].p_main != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > sibling->m_leaf.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > max_key);
    fdb_txpool_alloc_free(pvalsalloc, 
                          &tx, 
                          txtctx, 
                          sibling->m_leaf.m_leafs[i]);
  }

  fdb_txpool_alloc_free(pvalsalloc, 
                        &tx, 
                        txtctx, 
                        sibling->m_leaf.m_leafs[0]);

  fdb_txbtree_destroy_node(&btree, 
                           &tx, 
                           txtctx,
                           sibling_ref);

  fdb_txbtree_destroy_node(&btree, 
                           &tx,
                           txtctx, 
                           node_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_tx_commit(&tx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_release();
}


TEST(txbtree_test, fdb_btree_get) 
{
  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);


  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_internal(&btree, 
                                                                &tx, 
                                                                txtctx);
  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       node_ref, 
                                                                       true);

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    fdb_txpool_alloc_ref_t leaf_ref = fdb_txbtree_create_leaf(&btree, 
                                                              &tx, 
                                                              txtctx);

    fdb_txbtree_node_t* leaf = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                         &tx, 
                                                                         txtctx, 
                                                                         leaf_ref, 
                                                                         true);
    node->m_internal.m_children[i] = leaf_ref;
    for (uint32_t j = 0; j < FDB_TXBTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*FDB_TXBTREE_LEAF_MAX_ARITY+j;
      fdb_txpool_alloc_ref_t value_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                             &tx, 
                             txtctx, 
                             FDB_MIN_ALIGNMENT, 
                             sizeof(TestValue), 
                             FDB_NO_HINT);

      TestValue* value = (TestValue*)fdb_txpool_alloc_ptr(pvalsalloc, 
                                                          &tx, 
                                                          txtctx, 
                                                          value_ref, 
                                                          true);
      value->m_val = key;
      leaf->m_leaf.m_leafs[j] = value_ref;
      leaf->m_leaf.m_keys[j] = key; 
      leaf->m_leaf.m_nleafs++; 
    }
    node->m_internal.m_nchildren++;
    if( i >= 1 ) 
    {
      node->m_internal.m_keys[i-1] = leaf->m_leaf.m_keys[0];
    }
  }

  for( uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    for ( uint32_t j = 0; j < FDB_TXBTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*FDB_TXBTREE_LEAF_MAX_ARITY+j;
      fdb_txpool_alloc_ref_t value_ref = fdb_txbtree_get_node(&btree, 
                                                              &tx, 
                                                              txtctx, 
                                                              node_ref,
                                                              (i*FDB_TXBTREE_LEAF_MAX_ARITY+j));

      TestValue* value = (TestValue*)fdb_txpool_alloc_ptr(pvalsalloc, 
                                                          &tx, 
                                                          txtctx, 
                                                          value_ref, 
                                                          false);
      ASSERT_NE(value, nullptr);
      ASSERT_EQ(value->m_val, key);
      fdb_txpool_alloc_free(pvalsalloc, 
                            &tx,
                            txtctx, 
                            value_ref);
    }
  }

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

TEST(txbtree_test, fdb_btree_shift_insert_internal) 
{

  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_internal(&btree, 
                                                                &tx, 
                                                                txtctx);
  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                       &tx, 
                                                                       txtctx, 
                                                                       node_ref, 
                                                                       true);

  fdb_txpool_alloc_ref_t child_ref = fdb_txbtree_create_internal(&btree, 
                                                                 &tx, 
                                                                 txtctx);

  node->m_internal.m_children[0] = child_ref;
  node->m_internal.m_nchildren++;

  fdb_txpool_alloc_ref_t child2_ref = fdb_txbtree_create_internal(&btree, 
                                                                  &tx, 
                                                                  txtctx);
  fdb_txbtree_shift_insert_internal(&btree, 
                                    &tx, 
                                    txtctx, 
                                    node_ref, 
                                    1, 
                                    child2_ref, 
                                    10);

  ASSERT_EQ(node->m_internal.m_nchildren, 2);
  ASSERT_EQ(fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 node->m_internal.m_children[0], 
                                 false), 
            fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 child_ref, 
                                 false));

  ASSERT_EQ(fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 node->m_internal.m_children[1], 
                                 false), 
            fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 child2_ref, 
                                 false));
  ASSERT_EQ(node->m_internal.m_keys[0], 10);

  fdb_txpool_alloc_ref_t child3_ref = fdb_txbtree_create_internal(&btree, 
                                                                  &tx, 
                                                                  txtctx);
  fdb_txbtree_shift_insert_internal(&btree, 
                                    &tx, 
                                    txtctx, 
                                    node_ref, 
                                    1, 
                                    child3_ref, 
                                    5);

  ASSERT_EQ(node->m_internal.m_nchildren, 3);
  ASSERT_EQ(fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 node->m_internal.m_children[0], 
                                 false), 
            fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 child_ref, 
                                 false));
  ASSERT_EQ(fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 node->m_internal.m_children[1], 
                                 false), 
            fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 child3_ref, 
                                 false));
  ASSERT_EQ(fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 node->m_internal.m_children[2], 
                                 false), 
            fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                 &tx, 
                                 txtctx, 
                                 child2_ref, 
                                 false));
  ASSERT_EQ(node->m_internal.m_keys[0], 5);
  ASSERT_EQ(node->m_internal.m_keys[1], 10);

  fdb_txbtree_destroy_node(&btree,
                           &tx, 
                           txtctx, 
                           node_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(txbtree_test, fdb_btree_insert_root) 
{
  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t val1_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                                                           &tx, 
                                                           txtctx, 
                                                           FDB_MIN_ALIGNMENT, 
                                                           sizeof(TestValue), 
                                                           FDB_NO_HINT);
  fdb_txpool_alloc_ref_t val2_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                                                           &tx, 
                                                           txtctx, 
                                                           FDB_MIN_ALIGNMENT, 
                                                           sizeof(TestValue), 
                                                           FDB_NO_HINT);
  fdb_txbtree_insert_t insert = fdb_txbtree_insert(&btree, 
                                                   &tx, 
                                                   txtctx, 
                                                   0, 
                                                   val1_ref); 
  ASSERT_EQ(insert.m_inserted, true);

  insert = fdb_txbtree_insert(&btree, 
                              &tx, 
                              txtctx, 
                              1, 
                              val2_ref); 

  ASSERT_EQ(insert.m_inserted, true);

  insert =fdb_txbtree_insert(&btree,
                             &tx, 
                             txtctx, 
                             0, 
                             val1_ref); 
  ASSERT_EQ(insert.m_inserted, false);

  insert = fdb_txbtree_insert(&btree, 
                              &tx, 
                              txtctx, 
                              1, 
                              val2_ref); 
  ASSERT_EQ(insert.m_inserted, false);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_txpool_alloc_free(pvalsalloc, 
                        &tx, 
                        txtctx, 
                        val1_ref);

  fdb_txpool_alloc_free(pvalsalloc, 
                        &tx, 
                        txtctx, 
                        val2_ref);

  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);

  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx,
                           txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_release();
}

TEST(txbtree_test, fdb_btree_remove_shift_internal) 
{
  fdb_tx_init(NULL);
  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                           sizeof(TestValue), 
                                                           KILOBYTES(4),
                                                           NULL);
  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_internal(&btree, 
                                                                &tx, 
                                                                txtctx);
  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*) fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                        &tx, 
                                                                        txtctx, 
                                                                        node_ref, 
                                                                        true); 
  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = fdb_txbtree_create_internal(&btree, 
                                                                 &tx, 
                                                                 txtctx);
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    node->m_internal.m_keys[i] = (i+1)*10;
  }

  fdb_txbtree_remove_shift_internal(&btree, 
                                    &tx, 
                                    txtctx, 
                                    node_ref, 
                                    0);

  ASSERT_EQ(node->m_internal.m_nchildren, FDB_TXBTREE_INTERNAL_MAX_ARITY-1);

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_NE(node->m_internal.m_children[i].p_main, nullptr);
  }
  ASSERT_EQ(node->m_internal.m_children[FDB_TXBTREE_INTERNAL_MAX_ARITY-1].p_main, nullptr);

  for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    ASSERT_EQ(node->m_internal.m_keys[i], (i+2)*10);
  }

  fdb_txbtree_destroy_node(&btree, 
                           &tx, 
                           txtctx, 
                           node_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

TEST(txbtree_test, fdb_btree_remove_shift_leaf) 
{
  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                           sizeof(TestValue), 
                                                           KILOBYTES(4),
                                                           NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  fdb_txpool_alloc_ref_t node_ref = fdb_txbtree_create_leaf(&btree, 
                                                            &tx, 
                                                            txtctx);

  fdb_txbtree_node_t* node = (fdb_txbtree_node_t*) fdb_txpool_alloc_ptr(btree.p_factory->p_node_allocator, 
                                                                        &tx, 
                                                                        txtctx, 
                                                                        node_ref, 
                                                                        true);
  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
  {
    fdb_txpool_alloc_ref_t value_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                                                              &tx, 
                                                              txtctx, 
                                                              FDB_MIN_ALIGNMENT, 
                                                              sizeof(TestValue), 
                                                              FDB_NO_HINT);
    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx, 
                                                         txtctx, 
                                                         value_ref, 
                                                         true);
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value_ref;
    node->m_leaf.m_keys[i] = i*10;
    node->m_leaf.m_nleafs++;
  }

  fdb_txpool_alloc_ref_t value_ref = fdb_txbtree_remove_shift_leaf(&btree, 
                                                                   &tx, 
                                                                   txtctx, 
                                                                   node_ref, 
                                                                   0);
  ASSERT_EQ(node->m_leaf.m_nleafs, FDB_TXBTREE_LEAF_MAX_ARITY-1);
  ASSERT_NE(value_ref.p_main, nullptr);
  TestValue* value = (TestValue*)fdb_txpool_alloc_ptr(pvalsalloc, 
                                                      &tx, 
                                                      txtctx, 
                                                      value_ref, 
                                                      false);
  ASSERT_EQ(value->m_val, 0);
  fdb_txpool_alloc_free(pvalsalloc, 
                        &tx, 
                        txtctx, 
                        value_ref);

  for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY-1; ++i) 
  {
    fdb_txpool_alloc_ref_t value_ref = node->m_leaf.m_leafs[i];
    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx,
                                                         txtctx, 
                                                         value_ref, 
                                                         false);
    ASSERT_NE(value_ref.p_main, nullptr);
    ASSERT_EQ(value->m_val, i+1);
    ASSERT_EQ(node->m_leaf.m_keys[i], (i+1)*10);
    fdb_txpool_alloc_free(pvalsalloc, 
                          &tx, 
                          txtctx, 
                          value_ref);
  }
  ASSERT_EQ(node->m_leaf.m_leafs[FDB_TXBTREE_LEAF_MAX_ARITY-1].p_main, nullptr);

  fdb_txbtree_destroy_node(&btree,
                           &tx,
                           txtctx, 
                           node_ref);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_tx_commit(&tx);

  fdb_tx_begin(&tx, E_READ_WRITE);
  txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc,
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}


TEST(txbtree_test, BTIteratorTest) 
{
  fdb_tx_init(NULL);

  fdb_txpool_alloc_t* pvalsalloc = fdb_txpool_alloc_create(FDB_MIN_ALIGNMENT, 
                                                         sizeof(TestValue), 
                                                         KILOBYTES(4),
                                                         NULL);

  fdb_txbtree_factory_t btree_factory;
  fdb_txbtree_factory_init(&btree_factory, NULL);

  // We initialize the btree with a transaction
  fdb_tx_t tx;
  fdb_tx_begin(&tx, E_READ_WRITE);
  fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);

  fdb_txbtree_t btree;
  fdb_txbtree_init(&btree, 
                   &btree_factory, 
                   &tx, 
                   txtctx);

  uint32_t BTREE_MAX_BLOCK=1024;

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    fdb_txpool_alloc_ref_t value_ref = fdb_txpool_alloc_alloc(pvalsalloc, 
                                                              &tx, 
                                                              txtctx, 
                                                              FDB_MIN_ALIGNMENT, 
                                                              sizeof(TestValue), 
                                                              FDB_NO_HINT);

    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx, 
                                                         txtctx, 
                                                         value_ref, 
                                                         true);
    value->m_val = i;
    fdb_txbtree_insert_t insert = fdb_txbtree_insert(&btree, 
                                                     &tx, 
                                                     txtctx, 
                                                     value->m_val, 
                                                     value_ref);
    ASSERT_TRUE(insert.m_inserted);
  }

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    uint32_t val = i;
    fdb_txpool_alloc_ref_t value_ref = fdb_txbtree_get(&btree,
                                                       &tx, 
                                                       txtctx, 
                                                       val);

    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx, 
                                                         txtctx, 
                                                         value_ref, 
                                                         false);
    ASSERT_EQ(value->m_val, val);
  }

  fdb_txbtree_iter_t iterator;
  fdb_txbtree_iter_init(&iterator, 
                        &btree, 
                        &tx, 
                        txtctx);
  uint32_t val = 0;
  while (fdb_txbtree_iter_has_next(&iterator)) 
  {
    fdb_txpool_alloc_ref_t value_ref = fdb_txbtree_iter_next(&iterator).m_value_ref;

    TestValue* value = (TestValue*) fdb_txpool_alloc_ptr(pvalsalloc, 
                                                         &tx, 
                                                         txtctx, 
                                                         value_ref, 
                                                         false);
    ASSERT_EQ(value->m_val, val);
    fdb_txpool_alloc_free(pvalsalloc, 
                          &tx, 
                          txtctx, 
                          value_ref);
    val++;
  }
  fdb_txbtree_iter_release(&iterator);

  fdb_txbtree_release(&btree, &tx, txtctx);
  fdb_txbtree_factory_release(&btree_factory, 
                              &tx, 
                              txtctx);
  fdb_txpool_alloc_destroy(pvalsalloc, 
                           &tx, 
                           txtctx);
  fdb_tx_commit(&tx);
  fdb_tx_release();
}

/*
TEST(txbtree_test, BTreeSteps) 
{
  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, KILOBYTES(64), nullptr);
  fdb_btree_t btree;
  fdb_btree_init(&btree, &lallocator.m_super);
  constexpr uint32_t MAX_ELEMENTS = 1000;
  uint32_t stride = 21;
  uint32_t offset = 21604;
  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    fdb_btree_insert(&btree, i*stride + offset, (void*)((uint64_t)i*stride + offset));
  }

  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    void* ptr = fdb_btree_get(&btree, i*stride+offset);
    ASSERT_EQ(((uint64_t)ptr), i*stride+offset);
  }

  fdb_btree_release(&btree);
  fdb_stack_alloc_release(&lallocator);
  //ASSERT_EQ(fdb_btree_num_allocations, 0);
}
*/


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
