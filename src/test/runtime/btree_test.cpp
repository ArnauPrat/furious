
#include "../../common/hashtable.h"
#include "../../common/btree.h"
#include "../../common/types.h"
#include "../../common/memory/stack_allocator.h"

#include <gtest/gtest.h>

//extern int32_t fdb_btree_num_allocations;

struct TestValue 
{
  uint32_t m_val;
};

TEST(BTreeTest, fdb_btree_create) 
{
  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, 
                       KILOBYTES(4), 
                       nullptr);

  fdb_btree_t btree;
  fdb_btree_init(&btree, &lallocator.m_super);
  fdb_btree_node_t* internal = fdb_btree_create_internal(&btree);
  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(internal->m_internal.m_children[i], nullptr);
  }

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_EQ(internal->m_internal.m_keys[i], 0);
  }

  ASSERT_EQ(internal->m_internal.m_nchildren, 0);
  fdb_btree_destroy_node(&btree, internal);

  fdb_btree_node_t* leaf = fdb_btree_create_leaf(&btree);
  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(leaf->m_leaf.m_leafs[i], nullptr);
    ASSERT_EQ(leaf->m_leaf.m_keys[i], 0);
  }

  ASSERT_EQ(leaf->m_leaf.m_nleafs, 0);
  ASSERT_EQ(leaf->m_leaf.m_next, nullptr);
  fdb_btree_destroy_node(&btree,leaf);
  fdb_btree_release(&btree);
  fdb_stack_alloc_release(&lallocator);
}

TEST(BTreeTest, fdb_btree_next_internal) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, 
                 nullptr);
  fdb_btree_node_t* node = fdb_btree_create_internal(&btree);

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = fdb_btree_create_leaf(&btree);
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_keys[i-1] = i*10;
  }

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_btree_next_internal(node,i*10), i);
  }

  fdb_btree_destroy_node(&btree, node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_next_leaf) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_leaf(&btree);

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i*2;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i*2;
    node->m_leaf.m_nleafs++;
  }

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_btree_next_leaf(node,i*2), i);
  }

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(fdb_btree_next_leaf(node,i*2+1), i+1);
  }

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    delete (TestValue*)node->m_leaf.m_leafs[i];
  }

  fdb_btree_destroy_node(&btree, node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_split_internal) 
{

  fdb_btree_t btree;
  fdb_btree_init(&btree, 
                 nullptr);
  fdb_btree_node_t* node = fdb_btree_create_internal(&btree);

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = fdb_btree_create_leaf(&btree);
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_keys[i-1] = i*10;
  }

  uint32_t sibling_key;
  fdb_btree_node_t* sibling = fdb_btree_split_internal(&btree, 
                                               node, 
                                               &sibling_key);

  ASSERT_NE(node->m_internal.m_nchildren, 0);
  ASSERT_NE(sibling->m_internal.m_nchildren, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(node->m_internal.m_children[i] != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(sibling->m_internal.m_children[i] != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, FDB_BTREE_INTERNAL_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_internal.m_nchildren + sibling->m_internal.m_nchildren);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < FDB_BTREE_INTERNAL_MAX_ARITY -1 && node->m_internal.m_children[i+1] != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_internal.m_keys[i] > node->m_internal.m_keys[i-1]);
    max_key = node->m_internal.m_keys[i];
  }

  for (uint32_t i = 1; i < FDB_BTREE_INTERNAL_MAX_ARITY -1 && sibling->m_internal.m_children[i+1] != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > sibling->m_internal.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > max_key);
  }

  fdb_btree_destroy_node(&btree, 
                     sibling);
  fdb_btree_destroy_node(&btree, 
                     node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_split_leaf) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_leaf(&btree);

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  uint32_t sibling_key;
  fdb_btree_node_t* sibling = fdb_btree_split_leaf(&btree, 
                                           node, 
                                           &sibling_key);

  ASSERT_NE(node->m_leaf.m_nleafs, 0);
  ASSERT_NE(sibling->m_leaf.m_nleafs, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(node->m_leaf.m_leafs[i] != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(sibling->m_leaf.m_leafs[i] != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, FDB_BTREE_LEAF_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_leaf.m_nleafs + sibling->m_leaf.m_nleafs);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < FDB_BTREE_LEAF_MAX_ARITY && node->m_leaf.m_leafs[i] != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_leaf.m_keys[i] > node->m_leaf.m_keys[i-1]);
    max_key = node->m_leaf.m_keys[i];
    delete (TestValue*) node->m_leaf.m_leafs[i];
  }
  delete (TestValue*) node->m_leaf.m_leafs[0];

  for (uint32_t i = 1; i < FDB_BTREE_LEAF_MAX_ARITY && sibling->m_leaf.m_leafs[i] != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > sibling->m_leaf.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > max_key);
    delete (TestValue*) sibling->m_leaf.m_leafs[i];
  }
  delete (TestValue*) sibling->m_leaf.m_leafs[0];

  fdb_btree_destroy_node(&btree, 
                     sibling);
  fdb_btree_destroy_node(&btree, 
                     node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_get) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_internal(&btree);

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    fdb_btree_node_t* leaf = fdb_btree_create_leaf(&btree);
    node->m_internal.m_children[i] = leaf;
    for (uint32_t j = 0; j < FDB_BTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*FDB_BTREE_LEAF_MAX_ARITY+j;
      TestValue* value = new TestValue();
      value->m_val = key;
      leaf->m_leaf.m_leafs[j] = value;
      leaf->m_leaf.m_keys[j] = key; 
      leaf->m_leaf.m_nleafs++; 
    }
    node->m_internal.m_nchildren++;
    if( i >= 1 ) 
    {
      node->m_internal.m_keys[i-1] = leaf->m_leaf.m_keys[0];
    }
  }

  for( uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    for ( uint32_t j = 0; j < FDB_BTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*FDB_BTREE_LEAF_MAX_ARITY+j;
      TestValue* value = (TestValue*)fdb_btree_get_node(node, (i*FDB_BTREE_LEAF_MAX_ARITY+j));
      ASSERT_NE(value, nullptr);
      ASSERT_EQ(value->m_val, key);
      delete value;
    }
  }

  fdb_btree_destroy_node(&btree, node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_shift_insert_internal) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_internal(&btree);
  fdb_btree_node_t* child = fdb_btree_create_internal(&btree);
  node->m_internal.m_children[0] = child;
  node->m_internal.m_nchildren++;

  fdb_btree_node_t* child2 = fdb_btree_create_internal(&btree);
  fdb_btree_shift_insert_internal(node, 1, child2, 10);
  ASSERT_EQ(node->m_internal.m_nchildren, 2);
  ASSERT_EQ(node->m_internal.m_children[0], child);
  ASSERT_EQ(node->m_internal.m_children[1], child2);
  ASSERT_EQ(node->m_internal.m_keys[0], 10);

  fdb_btree_node_t* child3 = fdb_btree_create_internal(&btree);
  fdb_btree_shift_insert_internal(node, 1, child3, 5);
  ASSERT_EQ(node->m_internal.m_nchildren, 3);
  ASSERT_EQ(node->m_internal.m_children[0], child);
  ASSERT_EQ(node->m_internal.m_children[1], child3);
  ASSERT_EQ(node->m_internal.m_children[2], child2);
  ASSERT_EQ(node->m_internal.m_keys[0], 5);
  ASSERT_EQ(node->m_internal.m_keys[1], 10);

  fdb_btree_destroy_node(&btree, node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_insert_root) 
{
  fdb_btree_t root;
  fdb_btree_init(&root, nullptr);
  TestValue val1;
  TestValue val2;
  fdb_btree_insert_t insert = fdb_btree_insert(&root, 0, &val1); 
  ASSERT_EQ(insert.m_inserted, true);

  insert =fdb_btree_insert(&root, 1, &val2); 
  ASSERT_EQ(insert.m_inserted, true);

  insert =fdb_btree_insert(&root, 0, &val1); 
  ASSERT_EQ(insert.m_inserted, false);

  insert =fdb_btree_insert(&root, 1, &val2); 
  ASSERT_EQ(insert.m_inserted, false);

  fdb_btree_release(&root);
}

TEST(BTreeTest, fdb_btree_remove_shift_internal) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_internal(&btree);
  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = fdb_btree_create_internal(&btree);
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    node->m_internal.m_keys[i] = (i+1)*10;
  }

  fdb_btree_remove_shift_internal(&btree, node, 0);
  ASSERT_EQ(node->m_internal.m_nchildren, FDB_BTREE_INTERNAL_MAX_ARITY-1);

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_NE(node->m_internal.m_children[i], nullptr);
  }
  ASSERT_EQ(node->m_internal.m_children[FDB_BTREE_INTERNAL_MAX_ARITY-1], nullptr);

  for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    ASSERT_EQ(node->m_internal.m_keys[i], (i+2)*10);
  }

  fdb_btree_destroy_node(&btree, 
                     node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, fdb_btree_remove_shift_leaf) 
{
  fdb_btree_t btree;
  fdb_btree_init(&btree, nullptr);
  fdb_btree_node_t* node = fdb_btree_create_leaf(&btree);
  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i*10;
    node->m_leaf.m_nleafs++;
  }

  TestValue* value = (TestValue*)fdb_btree_remove_shift_leaf(node, 0);
  ASSERT_EQ(node->m_leaf.m_nleafs, FDB_BTREE_LEAF_MAX_ARITY-1);
  ASSERT_NE(value, nullptr);
  ASSERT_EQ(value->m_val, 0);
  delete value;

  for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY-1; ++i) 
  {
    TestValue* value = (TestValue*)node->m_leaf.m_leafs[i];
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->m_val, i+1);
    ASSERT_EQ(node->m_leaf.m_keys[i], (i+1)*10);
    delete value;
  }
  ASSERT_EQ(node->m_leaf.m_leafs[FDB_BTREE_LEAF_MAX_ARITY-1], nullptr);

  fdb_btree_destroy_node(&btree, node);
  fdb_btree_release(&btree);
}

TEST(BTreeTest, BTIteratorTest) 
{
  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, KILOBYTES(4), nullptr);
  fdb_btree_t btree;
  fdb_btree_init(&btree, &lallocator.m_super);

  uint32_t BTREE_MAX_BLOCK=1024;
  fdb_hashtable_t hashtable;
  fdb_hashtable_init(&hashtable, 512, nullptr);

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    TestValue* test_value = new TestValue{static_cast<uint32_t>(i)};
    fdb_hashtable_add(&hashtable, i, test_value);
  }

  fdb_hashtable_iter_t iter = fdb_hashtable_iter_init(&hashtable);
  while(fdb_hashtable_iter_has_next(&iter))
  {
    TestValue* next = (TestValue*)fdb_hashtable_iter_next(&iter).p_value;
    fdb_btree_insert_t insert = fdb_btree_insert(&btree, next->m_val, next);
    ASSERT_TRUE(insert.m_inserted);
  }
  fdb_hashtable_iter_release(&iter);

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    uint32_t val = i;
    TestValue* value =(TestValue*)fdb_btree_get(&btree, val);
    ASSERT_EQ(value->m_val, val);
  }

  fdb_btree_iter_t iterator;
  fdb_btree_iter_init(&iterator, 
                      &btree);
  uint32_t val = 0;
  while (fdb_btree_iter_has_next(&iterator)) 
  {
    TestValue* value = (TestValue*)fdb_btree_iter_next(&iterator).p_value;
    ASSERT_EQ(value->m_val, val);
    val++;
  }
  fdb_btree_iter_release(&iterator);

  iter = fdb_hashtable_iter_init(&hashtable);
  while(fdb_hashtable_iter_has_next(&iter))
  {
    TestValue* next = (TestValue*)fdb_hashtable_iter_next(&iter).p_value;
    delete next;
  }
  fdb_hashtable_iter_release(&iter);

  fdb_btree_release(&btree);
  fdb_hashtable_release(&hashtable);
  //ASSERT_EQ(fdb_btree_num_allocations, 0);
  fdb_stack_alloc_release(&lallocator);
}

TEST(BTreeTest, BTreeSteps) 
{
  fdb_stack_alloc_t lallocator;
  fdb_stack_alloc_init(&lallocator, KILOBYTES(4), nullptr);
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

/*
TEST(BTreeTest, BTree) 
{
  BTree<TestValue*>* btree = new BTree<TestValue*>();

  uint32_t BTREE_MAX_KEY=100000;

  TestValue* values[BTREE_MAX_KEY];

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) 
  {
    values[i] = new TestValue{static_cast<uint32_t>(i)};
    btree->insert_copy(static_cast<uint32_t>(i), &values[i]);
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) 
  {
    TestValue* value = *btree->get(i);
    ASSERT_EQ(value, values[i]);
  }

  BTree<TestValue*>::Iterator iterator = btree->iterator();
  uint32_t val = 0;
  while (iterator.has_next()) 
  {
    TestValue* value = *iterator.next().p_value;
    ASSERT_EQ(value->m_val, val);
    ASSERT_EQ(value, values[val]);
    val++;
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) 
  {
    btree->remove(i);
    delete values[i];
  }

  ASSERT_EQ(btree->size(), 0);

  // EVEN ELEMENTS
  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) 
  {
    values[i] = new TestValue{static_cast<uint32_t>(i)};
    btree->insert_copy(static_cast<uint32_t>(i), &values[i]);
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) 
  {
    TestValue* value = static_cast<TestValue*>(*btree->get(i));
    ASSERT_EQ(value, values[i]);
  }


  iterator = btree->iterator();
  val = 0;
  while (iterator.has_next()) 
  {
    TestValue* value = static_cast<TestValue*>(*iterator.next().p_value);
    ASSERT_EQ(value->m_val, val);
    ASSERT_EQ(value, values[val]);
    val+=2;
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) 
  {
    btree->remove(i);
    delete values[i];
  }

  ASSERT_EQ(btree->size(), 0);
  delete btree;

  //ASSERT_EQ(fdb_btree_num_allocations, 0);
}
*/

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
