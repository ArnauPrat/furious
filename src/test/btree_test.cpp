
#include <gtest/gtest.h>
#include "../data/btree.h"
#include "../data/impl/btree_impl.h"
#include "../common/common.h"
#include <iostream>

namespace furious {

extern int32_t btree_num_allocations;

constexpr uint8_t MAX_KEY = 255;
struct TestValue {
  uint8_t m_val;
};

TEST(BTreeTest, node_size) {
  ASSERT_EQ(sizeof(BTNode), 64);
}

TEST(BTreeTest, btree_create) {
  BTNode* internal = btree_create_internal();
  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    ASSERT_EQ(internal->m_internal.m_children[i], nullptr);
  }

  for (uint8_t i = 0; i < BTREE_MAX_ARITY-1; ++i) {
    ASSERT_EQ(internal->m_internal.m_keys[i], 0);
  }

  ASSERT_EQ(internal->m_internal.m_nchildren, 0);
  btree_destroy_node(internal);

  BTNode* leaf = btree_create_internal();
  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    ASSERT_EQ(leaf->m_leaf.m_leafs[i], nullptr);
    ASSERT_EQ(leaf->m_leaf.m_keys[i], 0);
  }

  ASSERT_EQ(leaf->m_leaf.m_nleafs, 0);
  ASSERT_EQ(leaf->m_leaf.m_next, nullptr);
  btree_destroy_node(leaf);
}

TEST(BTreeTest, btree_next_internal) {

  BTNode* node = btree_create_internal();

  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    node->m_internal.m_children[i] = btree_create_leaf();
    node->m_internal.m_nchildren++;
  }

  for (uint8_t i = 1; i < BTREE_MAX_ARITY; ++i) {
    node->m_internal.m_keys[i-1] = i*10;
  }

  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    ASSERT_EQ(btree_next_internal(node,i*10), i);
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_next_leaf) {

  BTNode* node = btree_create_leaf();

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    node->m_leaf.m_leafs[i] = new TestValue{i};
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    ASSERT_EQ(btree_next_leaf(node,i), i);
  }

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    delete static_cast<TestValue*>(node->m_leaf.m_leafs[i]);
  }
  btree_destroy_node(node);

}

TEST(BTreeTest, btree_split_internal) {

  BTNode* node = btree_create_internal();

  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    node->m_internal.m_children[i] = btree_create_leaf();
    node->m_internal.m_nchildren++;
  }

  for (uint8_t i = 1; i < BTREE_MAX_ARITY; ++i) {
    node->m_internal.m_keys[i-1] = i*10;
  }

  uint8_t sibling_key;
  BTNode* sibling = btree_split_internal(node, &sibling_key);

  ASSERT_NE(node->m_internal.m_nchildren, 0);
  ASSERT_NE(sibling->m_internal.m_nchildren, 0);

  uint32_t non_null = 0;
  for( uint8_t i = 0; i < BTREE_MAX_ARITY; ++i ) {
    if(node->m_internal.m_children[i] != nullptr) {
      non_null++;
    }
  }

  for( uint8_t i = 0; i < BTREE_MAX_ARITY; ++i ) {
    if(sibling->m_internal.m_children[i] != nullptr) {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, BTREE_MAX_ARITY);
  ASSERT_EQ(non_null, node->m_internal.m_nchildren + sibling->m_internal.m_nchildren);
  uint8_t max_key = 0;
  for (uint8_t i = 1; i < BTREE_MAX_ARITY -1 && node->m_internal.m_children[i+1] != nullptr; ++i) {
    ASSERT_TRUE(node->m_internal.m_keys[i] > node->m_internal.m_keys[i-1]);
    max_key = node->m_internal.m_keys[i];
  }

  for (uint8_t i = 1; i < BTREE_MAX_ARITY -1 && sibling->m_internal.m_children[i+1] != nullptr; ++i) {
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > sibling->m_internal.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_internal.m_keys[i] > max_key);
  }


  btree_destroy_node(sibling);
  btree_destroy_node(node);

}

TEST(BTreeTest, btree_split_leaf) {

  BTNode* node = btree_create_leaf();

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    node->m_leaf.m_leafs[i] = new TestValue{i};
    node->m_leaf.m_keys[i] = i;
    node->m_leaf.m_nleafs++;
  }

  uint8_t sibling_key;
  BTNode* sibling = btree_split_leaf(node, &sibling_key);

  ASSERT_NE(node->m_leaf.m_nleafs, 0);
  ASSERT_NE(sibling->m_leaf.m_nleafs, 0);

  uint32_t non_null = 0;
  for( uint8_t i = 0; i < BTREE_MIN_ARITY; ++i ) {
    if(node->m_leaf.m_leafs[i] != nullptr) {
      non_null++;
    }
  }

  for( uint8_t i = 0; i < BTREE_MIN_ARITY; ++i ) {
    if(sibling->m_leaf.m_leafs[i] != nullptr) {
      non_null++;
    }
  }

  ASSERT_EQ(non_null, BTREE_MIN_ARITY);
  ASSERT_EQ(non_null, node->m_leaf.m_nleafs + sibling->m_leaf.m_nleafs);
  uint8_t max_key = 0;
  for (uint8_t i = 1; i < BTREE_MIN_ARITY && node->m_leaf.m_leafs[i] != nullptr; ++i) {
    ASSERT_TRUE(node->m_leaf.m_keys[i] > node->m_leaf.m_keys[i-1]);
    max_key = node->m_leaf.m_keys[i];
  }

  for (uint8_t i = 1; i < BTREE_MIN_ARITY && sibling->m_leaf.m_leafs[i] != nullptr; ++i) {
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > sibling->m_leaf.m_keys[i-1]);
    ASSERT_TRUE(sibling->m_leaf.m_keys[i] > max_key);
  }

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    if (node->m_leaf.m_leafs[i] != nullptr) {
      delete static_cast<TestValue*>(node->m_leaf.m_leafs[i]);
    }
  }

  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    if (sibling->m_leaf.m_leafs[i] != nullptr) {
      delete static_cast<TestValue*>(sibling->m_leaf.m_leafs[i]);
    }
  }

  btree_destroy_node(sibling);
  btree_destroy_node(node);

}

TEST(BTreeTest, btree_get) {
 
  BTNode* node = btree_create_internal();

  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    BTNode* leaf = btree_create_leaf();
    node->m_internal.m_children[i] = leaf;
    for (uint8_t j = 0; j < BTREE_MIN_ARITY; ++j) {
      uint8_t key = i*BTREE_MIN_ARITY+j;
      leaf->m_leaf.m_leafs[j] = new TestValue{key};
      leaf->m_leaf.m_keys[j] = key; 
      leaf->m_leaf.m_nleafs++; 
    }
    node->m_internal.m_nchildren++;
    if( i >= 1 ) {
      node->m_internal.m_keys[i-1] = leaf->m_leaf.m_keys[0];
    }
  }

  for( uint8_t i = 0; i < BTREE_MAX_ARITY; ++i ) {
    for ( uint8_t j = 0; j < BTREE_MIN_ARITY; ++j) {
      uint8_t key = i*BTREE_MIN_ARITY+j;
      TestValue* value = static_cast<TestValue*>(btree_get(node, (i*BTREE_MIN_ARITY+j)));
      ASSERT_NE(value, nullptr);
      ASSERT_EQ(value->m_val, key);
    }
  }

  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    BTNode* leaf = node->m_internal.m_children[i];
    for (uint8_t j = 0; j < BTREE_MIN_ARITY; ++j) {
      delete static_cast<TestValue*>(leaf->m_leaf.m_leafs[j]);
    }
  }

  btree_destroy_node(node);
}

TEST(BTreeTest, btree_shift_insert_internal) {

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

TEST(BTreeTest, btree_shift_insert_leaf) {

  BTNode* node = btree_create_leaf();
  TestValue* child = new TestValue{0};
  node->m_leaf.m_leafs[0] = child;
  node->m_leaf.m_keys[0] = 0;
  node->m_leaf.m_nleafs++;

  TestValue* child2 = new TestValue{10};
  btree_shift_insert_leaf(node, 1, child2, 10);
  ASSERT_NE(node->m_leaf.m_leafs[0], nullptr);
  ASSERT_NE(node->m_leaf.m_leafs[1], nullptr);
  ASSERT_EQ(node->m_leaf.m_keys[0], 0);
  ASSERT_EQ(node->m_leaf.m_keys[1], 10);
  ASSERT_EQ(static_cast<TestValue*>(node->m_leaf.m_leafs[0])->m_val, 0 );
  ASSERT_EQ(static_cast<TestValue*>(node->m_leaf.m_leafs[1])->m_val, 10 );

  TestValue* child3 = new TestValue{5};
  btree_shift_insert_leaf(node, 1, child3, 5);

  ASSERT_NE(node->m_leaf.m_leafs[0], nullptr);
  ASSERT_NE(node->m_leaf.m_leafs[1], nullptr);
  ASSERT_NE(node->m_leaf.m_leafs[1], nullptr);
  ASSERT_EQ(node->m_leaf.m_keys[0], 0);
  ASSERT_EQ(node->m_leaf.m_keys[1], 5);
  ASSERT_EQ(node->m_leaf.m_keys[2], 10);
  ASSERT_EQ(static_cast<TestValue*>(node->m_leaf.m_leafs[0])->m_val, 0 );
  ASSERT_EQ(static_cast<TestValue*>(node->m_leaf.m_leafs[1])->m_val, 5 );
  ASSERT_EQ(static_cast<TestValue*>(node->m_leaf.m_leafs[2])->m_val, 10 );

  delete child;
  delete child2;
  delete child3;
  btree_destroy_node(node);
  
}

TEST(BTreeTest, btree_insert) {

  BTNode* node = btree_create_internal();
  TestValue* element = new TestValue{0};
  btree_insert(node, 0, element );
  ASSERT_NE(node->m_internal.m_children[0], nullptr);
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_leafs[0], element);
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_keys[0], 0);

  TestValue* element2 = new TestValue{1};
  btree_insert(node, 1, element2 );
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_leafs[1], element2);
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_keys[1], 1);

  TestValue* element3 = new TestValue{2};
  btree_insert(node, 2, element3 );
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_leafs[2], element3);
  ASSERT_EQ(node->m_internal.m_children[0]->m_leaf.m_keys[2], 2);

  delete element;
  delete element2;
  delete element3;
  btree_destroy_node(node);
  
}

TEST(BTreeTest, btree_remove_shift_internal) {

  BTNode* node = btree_create_internal();
  for (uint8_t i = 0; i < BTREE_MAX_ARITY; ++i) {
    node->m_internal.m_children[i] = btree_create_internal();
    node->m_internal.m_nchildren++;
  }

  for (uint8_t i = 0; i < BTREE_MAX_ARITY-1; ++i) {
    node->m_internal.m_keys[i] = (i+1)*10;
  }

  btree_remove_shift_internal(node, 0);
  ASSERT_EQ(node->m_internal.m_nchildren, BTREE_MAX_ARITY-1);

  for (uint8_t i = 0; i < BTREE_MAX_ARITY-1; ++i) {
    ASSERT_NE(node->m_internal.m_children[i], nullptr);
  }
  ASSERT_EQ(node->m_internal.m_children[BTREE_MAX_ARITY-1], nullptr);

  for (uint8_t i = 0; i < BTREE_MAX_ARITY-2; ++i) {
    ASSERT_EQ(node->m_internal.m_keys[i], (i+2)*10);
  }

  btree_destroy_node(node);

}

TEST(BTreeTest, btree_remove_shift_leaf) {

  BTNode* node = btree_create_leaf();
  for (uint8_t i = 0; i < BTREE_MIN_ARITY; ++i) {
    node->m_leaf.m_leafs[i] = new TestValue{i};
    node->m_leaf.m_keys[i] = i*10;
    node->m_leaf.m_nleafs++;
  }

  delete static_cast<TestValue*>(node->m_leaf.m_leafs[0]);
  btree_remove_shift_leaf(node, 0);
  ASSERT_EQ(node->m_leaf.m_nleafs, BTREE_MIN_ARITY-1);

  for (uint8_t i = 0; i < BTREE_MIN_ARITY-1; ++i) {
    ASSERT_NE(node->m_leaf.m_leafs[i], nullptr);
    ASSERT_EQ(node->m_leaf.m_keys[i], (i+1)*10);
  }
  ASSERT_EQ(node->m_leaf.m_leafs[BTREE_MIN_ARITY-1], nullptr);

  for (uint8_t i = 0; i < BTREE_MIN_ARITY-1; ++i) {
    delete static_cast<TestValue*>(node->m_leaf.m_leafs[i]);
  }

  btree_destroy_node(node);

}

TEST(BTreeTest, BTree) {
  BTree<TestValue>* btree = new BTree<TestValue>();

  uint32_t BTREE_MAX_KEY=255;

  TestValue* values[BTREE_MAX_KEY];

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) {
    values[i] = new TestValue{static_cast<uint8_t>(i)};
    btree->insert(static_cast<uint8_t>(i), values[i]);
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) {
    TestValue* value = btree->get(i);
    ASSERT_EQ(value, values[i]);
  }

  BTree<TestValue>::Iterator* iterator = btree->iterator();
  uint32_t val = 0;
  while (iterator->has_next()) {
    TestValue* value = iterator->next();
    ASSERT_EQ(value->m_val, val);
    ASSERT_EQ(value, values[val]);
    val++;
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; ++i) {
    TestValue* value = btree->remove(i);
    ASSERT_EQ(value, values[i]);
    ASSERT_EQ(value->m_val, static_cast<uint8_t>(i));
    delete value;
  }

  ASSERT_EQ(btree->size(), 0);

  // EVEN ELEMENTS
  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) {
    values[i] = new TestValue{static_cast<uint8_t>(i)};
    btree->insert(static_cast<uint8_t>(i), values[i]);
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) {
    TestValue* value = static_cast<TestValue*>(btree->get(i));
    ASSERT_EQ(value, values[i]);
  }


  delete iterator;

  iterator = btree->iterator();
  val = 0;
  while (iterator->has_next()) {
    TestValue* value = static_cast<TestValue*>(iterator->next());
    ASSERT_EQ(value->m_val, val);
    ASSERT_EQ(value, values[val]);
    val+=2;
  }

  for (uint32_t i = 0; i <= BTREE_MAX_KEY; i+=2) {
    TestValue* value = btree->remove(i);
    ASSERT_EQ(value, values[i]);
    ASSERT_EQ(value->m_val, static_cast<uint8_t>(i));
    delete value;
  }

  ASSERT_EQ(btree->size(), 0);
  delete iterator;
  delete btree;

  ASSERT_EQ(btree_num_allocations, 0);
}

} /* furious */ 

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
