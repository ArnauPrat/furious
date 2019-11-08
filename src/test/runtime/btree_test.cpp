
#include <gtest/gtest.h>
#include "../../common/btree.h"
#include "../../common/types.h"
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
  BTNode* internal = btree_create_internal();
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

  BTNode* leaf = btree_create_leaf();
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
  BTNode* node = btree_create_internal();

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
  BTNode* node = btree_create_leaf();

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    ASSERT_EQ(btree_next_leaf(node,i), i);
  }

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    delete (TestValue*)node->m_leaf.m_leafs[i];
  }

  btree_destroy_node(node);

}

TEST(BTreeTest, btree_split_internal) 
{

  BTNode* node = btree_create_internal();

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
  BTNode* sibling = btree_split_internal(node, &sibling_key);

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
  BTNode* node = btree_create_leaf();

  for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
  {
    TestValue* value = new TestValue();
    value->m_val = i;
    node->m_leaf.m_leafs[i] = value;
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  uint32_t sibling_key;
  BTNode* sibling = btree_split_leaf(node, &sibling_key);

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
 
  BTNode* node = btree_create_internal();

  for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    BTNode* leaf = btree_create_leaf();
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
      TestValue* value = (TestValue*)btree_get(node, (i*BTREE_LEAF_MAX_ARITY+j));
      ASSERT_NE(value, nullptr);
      ASSERT_EQ(value->m_val, key);
      delete value;
    }
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_shift_insert_internal) 
{
  BTNode* node = btree_create_internal();
  BTNode* child = btree_create_internal();
  node->m_internal.m_children[0] = child;
  node->m_internal.m_nchildren++;

  BTNode* child2 = btree_create_internal();
  btree_shift_insert_internal(node, 1, child2, 10);
  ASSERT_EQ(node->m_internal.m_nchildren, 2);
  ASSERT_EQ(node->m_internal.m_children[0], child);
  ASSERT_EQ(node->m_internal.m_children[1], child2);
  ASSERT_EQ(node->m_internal.m_keys[0], 10);

  BTNode* child3 = btree_create_internal();
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
  BTRoot* root = btree_create_root();
  TestValue val1;
  TestValue val2;
  BTInsert insert =btree_insert_root(root, 0); 
  ASSERT_EQ(insert.m_inserted, true);
  *insert.p_place = &val1;

  insert =btree_insert_root(root, 1); 
  ASSERT_EQ(insert.m_inserted, true);
  *insert.p_place = &val2;

  insert =btree_insert_root(root, 0); 
  ASSERT_EQ(insert.m_inserted, false);
  ASSERT_EQ(*insert.p_place, &val1);

  insert =btree_insert_root(root, 1); 
  ASSERT_EQ(insert.m_inserted, false);
  ASSERT_EQ(*insert.p_place, &val2);

  btree_destroy_root(root);
}

TEST(BTreeTest, btree_remove_shift_internal) 
{
  BTNode* node = btree_create_internal();
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
  BTNode* node = btree_create_leaf();
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
