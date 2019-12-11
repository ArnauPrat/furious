
#include <gtest/gtest.h>
#include "../../common/btree.h"
#include "../../common/types.h"
#include "../../common/hashtable.h"
#include <iostream>

namespace furious 
{

//extern int32_t btree_num_allocations;

struct TestValue 
{
  uint32_t m_val;
};

TEST(BTreeTest, btree_create) 
{
  btree_node_t* internal = btree_create_internal();
  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(internal->m_internal.m_children[i], nullptr);
  }

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_EQ(internal->m_internal.m_keys[i], 0);
  }

  ASSERT_EQ(internal->m_internal.m_nchildren, 0);
  btree_destroy_node(internal);

  btree_node_t* leaf = btree_create_leaf();
  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(leaf->m_leaf.m_leafs[i], nullptr);
    ASSERT_EQ(leaf->m_leaf.m_keys[i], 0);
  }

  ASSERT_EQ(leaf->m_leaf.m_nleafs, 0);
  ASSERT_EQ(leaf->m_leaf.m_next, nullptr);
  btree_destroy_node(leaf);
}

TEST(BTreeTest, btree_next_internal) 
{
  btree_node_t* node = btree_create_internal();

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = btree_create_leaf();
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_keys[i-1] = i*10;
  }

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(btree_next_internal(node,i*10), i);
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_next_leaf) 
{
  btree_node_t* node = btree_create_leaf();

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i*3;
    node->m_leaf.m_nleafs++;
  }

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(btree_next_leaf(node,i*3), i);
  }

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(btree_next_leaf(node,i*3-(i != 0 ? 1 : 0)), i);
  }

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    delete (TestValue*)node->m_leaf.m_leafs[i];
  }

  btree_destroy_node(node);

}

TEST(BTreeTest, btree_split_internal) 
{

  btree_node_t* node = btree_create_internal();

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = btree_create_leaf();
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 1; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_keys[i-1] = i*10;
  }

  uint32_t sibling_key;
  btree_node_t* sibling = btree_split_internal(node, &sibling_key);

  ASSERT_NE(node->m_internal.m_nchildren, 0);
  ASSERT_NE(sibling->m_internal.m_nchildren, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(node->m_internal.m_children[i] != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    if(sibling->m_internal.m_children[i] != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, BTREE_INTERNAL_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_internal.m_nchildren + sibling->m_internal.m_nchildren);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < BTREE_INTERNAL_MAX_ARITY -1 && node->m_internal.m_children[i+1] != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_internal.m_keys[i] > node->m_internal.m_keys[i-1]);
    max_key = node->m_internal.m_keys[i];
  }

  for (uint32_t i = 1; i < BTREE_INTERNAL_MAX_ARITY -1 && sibling->m_internal.m_children[i+1] != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > sibling->m_internal.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > max_key);
  }

  btree_destroy_node(sibling);
  btree_destroy_node(node);
}

TEST(BTreeTest, btree_split_leaf) 
{
  btree_node_t* node = btree_create_leaf();

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  uint32_t sibling_key;
  btree_node_t* sibling = btree_split_leaf(node, &sibling_key);

  ASSERT_NE(node->m_leaf.m_nleafs, 0);
  ASSERT_NE(sibling->m_leaf.m_nleafs, 0);

  uint32_t non_null = 0;
  for( uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(node->m_leaf.m_leafs[i] != nullptr) 
    {
      non_null++;
    }
  }

  for( uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    if(sibling->m_leaf.m_leafs[i] != nullptr) 
    {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, BTREE_LEAF_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_leaf.m_nleafs + sibling->m_leaf.m_nleafs);
  uint32_t max_key = 0;
  for (uint32_t i = 1; i < BTREE_LEAF_MAX_ARITY && node->m_leaf.m_leafs[i] != nullptr; ++i) 
  {
    ASSERT_TRUE(node->m_leaf.m_keys[i] > node->m_leaf.m_keys[i-1]);
    max_key = node->m_leaf.m_keys[i];
    delete (TestValue*) node->m_leaf.m_leafs[i];
  }
  delete (TestValue*) node->m_leaf.m_leafs[0];

  for (uint32_t i = 1; i < BTREE_LEAF_MAX_ARITY && sibling->m_leaf.m_leafs[i] != nullptr; ++i) 
  {
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > sibling->m_leaf.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > max_key);
    delete (TestValue*) sibling->m_leaf.m_leafs[i];
  }
  delete (TestValue*) sibling->m_leaf.m_leafs[0];

  btree_destroy_node(sibling);
  btree_destroy_node(node);
}

TEST(BTreeTest, btree_get) 
{
 
  btree_node_t* node = btree_create_internal();

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    btree_node_t* leaf = btree_create_leaf();
    node->m_internal.m_children[i] = leaf;
    for (uint32_t j = 0; j < BTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*BTREE_LEAF_MAX_ARITY+j;
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

  for( uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i ) 
  {
    for ( uint32_t j = 0; j < BTREE_LEAF_MAX_ARITY; ++j) 
    {
      uint32_t key = i*BTREE_LEAF_MAX_ARITY+j;
      TestValue* value = (TestValue*)btree_get_node(node, (i*BTREE_LEAF_MAX_ARITY+j));
      ASSERT_NE(value, nullptr);
      ASSERT_EQ(value->m_val, key);
      delete value;
    }
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_shift_insert_internal) 
{
  btree_node_t* node = btree_create_internal();
  btree_node_t* child = btree_create_internal();
  node->m_internal.m_children[0] = child;
  node->m_internal.m_nchildren++;

  btree_node_t* child2 = btree_create_internal();
  btree_shift_insert_internal(node, 1, child2, 10);
  ASSERT_EQ(node->m_internal.m_nchildren, 2);
  ASSERT_EQ(node->m_internal.m_children[0], child);
  ASSERT_EQ(node->m_internal.m_children[1], child2);
  ASSERT_EQ(node->m_internal.m_keys[0], 10);

  btree_node_t* child3 = btree_create_internal();
  btree_shift_insert_internal(node, 1, child3, 5);
  ASSERT_EQ(node->m_internal.m_nchildren, 3);
  ASSERT_EQ(node->m_internal.m_children[0], child);
  ASSERT_EQ(node->m_internal.m_children[1], child3);
  ASSERT_EQ(node->m_internal.m_children[2], child2);
  ASSERT_EQ(node->m_internal.m_keys[0], 5);
  ASSERT_EQ(node->m_internal.m_keys[1], 10);

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_insert_root) 
{
  btree_t root = {0};
  btree_init(&root);
  TestValue val1;
  TestValue val2;
  btree_insert_t insert = btree_insert(&root, 0); 
  ASSERT_EQ(insert.m_inserted, true);
  *insert.p_place = &val1;

  insert =btree_insert(&root, 1); 
  ASSERT_EQ(insert.m_inserted, true);
  *insert.p_place = &val2;

  insert =btree_insert(&root, 0); 
  ASSERT_EQ(insert.m_inserted, false);
  ASSERT_EQ(*insert.p_place, &val1);

  insert =btree_insert(&root, 1); 
  ASSERT_EQ(insert.m_inserted, false);
  ASSERT_EQ(*insert.p_place, &val2);

  btree_release(&root);
}

TEST(BTreeTest, btree_remove_shift_internal) 
{
  btree_node_t* node = btree_create_internal();
  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    node->m_internal.m_children[i] = btree_create_internal();
    node->m_internal.m_nchildren++;
  }

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    node->m_internal.m_keys[i] = (i+1)*10;
  }

  btree_remove_shift_internal(node, 0);
  ASSERT_EQ(node->m_internal.m_nchildren, BTREE_INTERNAL_MAX_ARITY-1);

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  {
    ASSERT_NE(node->m_internal.m_children[i], nullptr);
  }
  ASSERT_EQ(node->m_internal.m_children[BTREE_INTERNAL_MAX_ARITY-1], nullptr);

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    ASSERT_EQ(node->m_internal.m_keys[i], (i+2)*10);
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_remove_shift_leaf) 
{
  btree_node_t* node = btree_create_leaf();
  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i*10;
    node->m_leaf.m_nleafs++;
  }

  TestValue* value = (TestValue*)btree_remove_shift_leaf(node, 0);
  ASSERT_EQ(node->m_leaf.m_nleafs, BTREE_LEAF_MAX_ARITY-1);
  ASSERT_NE(value, nullptr);
  ASSERT_EQ(value->m_val, 0);
  delete value;

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY-1; ++i) 
  {
    TestValue* value = (TestValue*)node->m_leaf.m_leafs[i];
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(value->m_val, i+1);
    ASSERT_EQ(node->m_leaf.m_keys[i], (i+1)*10);
    delete value;
  }
  ASSERT_EQ(node->m_leaf.m_leafs[BTREE_LEAF_MAX_ARITY-1], nullptr);

  btree_destroy_node(node);
}

TEST(BTreeTest, BTIteratorTest) 
{
  btree_t btree;
  btree_init(&btree);

  uint32_t BTREE_MAX_BLOCK=1024;
  hashtable_t hashtable;
  hashtable_init(&hashtable, 512);

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    TestValue* test_value = new TestValue{static_cast<uint32_t>(i)};
    hashtable_add(&hashtable, i, test_value);
  }

  hashtable_iter_t iter;
  hashtable_iter_init(&iter, &hashtable);
  while(hashtable_iter_has_next(&iter))
  {
    TestValue* next = (TestValue*)hashtable_iter_next(&iter).p_value;
    btree_insert_t insert = btree_insert(&btree, next->m_val);
    ASSERT_TRUE(insert.m_inserted);
    *insert.p_place = next;
  }
  hashtable_iter_release(&iter);

  for (uint32_t i = 0; i < BTREE_MAX_BLOCK; ++i) 
  {
    uint32_t val = i;
    TestValue* value =(TestValue*)btree_get(&btree, val);
    ASSERT_EQ(value->m_val, val);
  }

  BTIterator iterator(&btree);
  uint32_t val = 0;
  while (iterator.has_next()) 
  {
    TestValue* value = (TestValue*)iterator.next().p_value;
    ASSERT_EQ(value->m_val, val);
    val++;
  }

  hashtable_iter_init(&iter, &hashtable);
  while(hashtable_iter_has_next(&iter))
  {
    TestValue* next = (TestValue*)hashtable_iter_next(&iter).p_value;
    delete next;
  }
  hashtable_iter_release(&iter);

  btree_release(&btree);
  //ASSERT_EQ(btree_num_allocations, 0);
}

TEST(BTreeTest, BTreeSteps) 
{
  btree_t btree;
  btree_init(&btree);
  constexpr uint32_t MAX_ELEMENTS = 1000;
  uint32_t stride = 21;
  uint32_t offset = 21604;
  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    btree_insert_t insert = btree_insert(&btree, i*stride + offset);
    void* val = (void*)((uint64_t)i*stride + offset);
    *insert.p_place = val;
  }

  for (uint32_t i = 0; i < MAX_ELEMENTS; ++i) 
  {
    void* ptr = btree_get(&btree, i*stride+offset);
    ASSERT_EQ(((uint64_t)ptr), i*stride+offset);
  }

  btree_release(&btree);
  //ASSERT_EQ(btree_num_allocations, 0);
}

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

  //ASSERT_EQ(btree_num_allocations, 0);
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
