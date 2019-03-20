
#include "btree_impl.h"
#include <string.h>
#include <cassert>
#include <iostream>

namespace furious 
{

#define FURIOUS_BTREE_GROWTH_FACTOR 128

BTIterator::BTIterator(BTRoot* root) : 
m_root(root), 
m_leaf(nullptr), 
m_index(0) 
{
  m_leaf = root->p_root;
  while(m_leaf != nullptr && 
        m_leaf->m_type == BTNodeType::E_INTERNAL) 
  {
    m_leaf = m_leaf->m_internal.m_children[0];
  }
}

bool 
BTIterator::has_next() const
{
  if(m_leaf == nullptr) 
  {
    return false;
  }
  if (m_index < m_leaf->m_leaf.m_nleafs) 
  {
    return true;
  } 
  BTNode* sibling = m_leaf->m_leaf.m_next;
  if(sibling == nullptr) 
  {
    return false;
  }
  return sibling->m_leaf.m_nleafs > 0;
}

BTEntry
BTIterator::next() 
{
  if(m_leaf == nullptr || m_leaf->m_leaf.m_nleafs == 0) 
  {
    return BTEntry{0xffffffff,nullptr};
  }
  uint32_t id = m_leaf->m_leaf.m_keys[m_index];
  void* value = m_leaf->m_leaf.m_leafs[m_index];
  m_index+=1;
  if(m_index >= m_leaf->m_leaf.m_nleafs) 
  {
    m_index = 0;
    m_leaf = m_leaf->m_leaf.m_next;
  }
  return BTEntry{id, value};
}

int32_t btree_num_allocations = 0;

BTNode* 
btree_create_internal() 
{
  BTNode* node = new BTNode();
  node->m_type = BTNodeType::E_INTERNAL;
  memset(&node->m_internal.m_children[0],0,sizeof(BTNode*)*BTREE_INTERNAL_MAX_ARITY);
  memset(&node->m_internal.m_keys[0],0,sizeof(uint32_t)*(BTREE_INTERNAL_MAX_ARITY-1));
  node->m_internal.m_nchildren = 0;
  btree_num_allocations++;
  return node;
}

BTNode* 
btree_create_leaf() 
{
  BTNode* node = new BTNode();
  node->m_type = BTNodeType::E_LEAF;
  memset(&node->m_leaf.m_leafs[0],0,sizeof(void*)*BTREE_LEAF_MAX_ARITY);
  memset(&node->m_leaf.m_keys[0],0,sizeof(uint32_t)*BTREE_LEAF_MAX_ARITY);
  node->m_leaf.m_next = 0;
  node->m_leaf.m_nleafs = 0;
  btree_num_allocations++;
  return node;
}

BTRoot*
btree_create_root()
{
  BTRoot* root = new BTRoot();
  root->p_root = btree_create_internal();
  return root;
}

void
btree_destroy_root(BTRoot* root)
{
  if(root->p_root != nullptr)
  {
    btree_destroy_node(root->p_root);
    root->p_root = nullptr;
  }
  delete root;
}

void 
btree_destroy_node(BTNode* node) 
{
  if (node->m_type == BTNodeType::E_INTERNAL) 
  {
    for (uint32_t i = 0; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
    {
      if (node->m_internal.m_children[i] != nullptr) 
      {
        btree_destroy_node(node->m_internal.m_children[i]);
        node->m_internal.m_children[i] = nullptr;
      }
    }
  } 
  else
  {
    for (uint32_t i = 0; i < BTREE_LEAF_MAX_ARITY; ++i) 
    {
      if (node->m_leaf.m_leafs[i] != nullptr) 
      {
        node->m_leaf.m_leafs[i] = nullptr;
      }
    }
  }
  btree_num_allocations--;
  delete node;
}

uint32_t 
btree_next_internal(BTNode* node, uint32_t key) 
{
  if(node->m_internal.m_nchildren <= 1) 
  {
    return 0;
  }

  uint32_t i = 0;
  for (; i < BTREE_INTERNAL_MAX_ARITY-1 && 
         node->m_internal.m_children[i+1] != nullptr && 
         key >= node->m_internal.m_keys[i]; 
      ++i);
  return i;
}

uint32_t 
btree_next_leaf(BTNode* node, 
                uint32_t key) 
{
  uint32_t i = 0;
  for (; i < BTREE_LEAF_MAX_ARITY && 
         node->m_leaf.m_leafs[i] != nullptr && 
         key > node->m_leaf.m_keys[i]; 
      ++i);
  return i;
}

void*
btree_get(BTNode* node, 
          uint32_t ekey) 
{
  if(node->m_type == BTNodeType::E_INTERNAL) 
  {
    uint32_t child_idx = btree_next_internal(node, ekey);
    BTNode* child = node->m_internal.m_children[child_idx];
    if( child != nullptr ) 
    { // if we found such a children, then proceed. If nullptr, means the node is empty
      return btree_get(child, ekey);
    }
  } 
  else 
  { // LEAF NODE
    uint32_t i = btree_next_leaf(node, ekey);
    if(node->m_leaf.m_keys[i] == ekey && 
       node->m_leaf.m_leafs[i] != nullptr)
    {
      return node->m_leaf.m_leafs[i];
    }
  }
  return nullptr;
}

void* 
btree_get_root(BTRoot* root, 
               uint32_t ekey)
{
  void* value = btree_get(root->p_root, ekey);
  if(value != nullptr)
  {
    return value;
  }
  return nullptr;
}


BTNode* 
btree_split_internal(BTNode* node, 
                     uint32_t* sibling_key) 
{
  BTNode* sibling = btree_create_internal();
  uint32_t start = BTREE_INTERNAL_MIN_ARITY;
  *sibling_key = node->m_internal.m_keys[start-1];
  node->m_internal.m_keys[start-1] = 0;
  for(uint32_t i = start; i < BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    sibling->m_internal.m_children[i-start] = node->m_internal.m_children[i];
    sibling->m_internal.m_nchildren++;
    node->m_internal.m_children[i] = nullptr;
    node->m_internal.m_nchildren--;
  }
  for(uint32_t i = start; i < BTREE_INTERNAL_MAX_ARITY-1; ++i ) 
  {
    sibling->m_internal.m_keys[i-start] = node->m_internal.m_keys[i];
    node->m_internal.m_keys[i] = 0;
  }
  return sibling;
}

BTNode* 
btree_split_leaf(BTNode* node, 
                 uint32_t* sibling_key) 
{
  BTNode* sibling = btree_create_leaf();
  uint32_t start = BTREE_LEAF_MIN_ARITY;
  *sibling_key = node->m_leaf.m_keys[start];
  for(uint32_t i = start; i < BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    sibling->m_leaf.m_leafs[i-start] = node->m_leaf.m_leafs[i];
    sibling->m_leaf.m_keys[i-start] = node->m_leaf.m_keys[i];
    sibling->m_leaf.m_nleafs++;
    node->m_leaf.m_leafs[i] = nullptr;
    node->m_leaf.m_nleafs--;
  }
  return sibling;
}

void 
btree_shift_insert_internal(BTNode* node, 
                            uint32_t idx, 
                            BTNode* child, 
                            uint32_t key ) 
{
  assert(idx > 0);
  for(uint32_t i = BTREE_INTERNAL_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_internal.m_children[i] = node->m_internal.m_children[i-1];
  }

  for(uint32_t i = BTREE_INTERNAL_MAX_ARITY-2; i > idx-1; --i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i-1];
  }
  node->m_internal.m_children[idx] = child;
  node->m_internal.m_keys[idx-1] = key;
  node->m_internal.m_nchildren++;
}

void 
btree_shift_insert_leaf(BTRoot* root,
                        BTNode* node, 
                        uint32_t idx, 
                        uint32_t key ) 
{
  for(uint32_t i = BTREE_LEAF_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i-1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i-1];
  }

  node->m_leaf.m_leafs[idx] = nullptr;
  node->m_leaf.m_keys[idx] = key;
  node->m_leaf.m_nleafs++;
}


BTNode* 
btree_split_child_full(BTNode* node, uint32_t child_idx, uint32_t key)
{
  BTNode* child = node->m_internal.m_children[child_idx];
  if(child->m_type == BTNodeType::E_INTERNAL && child->m_internal.m_nchildren == BTREE_INTERNAL_MAX_ARITY) 
  {
    uint32_t sibling_key;
    BTNode* sibling = btree_split_internal(child, &sibling_key);
    btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
    if(key >= sibling_key) 
    {
      child = sibling;
    }
  } 
  else if( child->m_type == BTNodeType::E_LEAF && child->m_leaf.m_nleafs == BTREE_LEAF_MAX_ARITY )
  {
    uint32_t sibling_key;
    BTNode* sibling = btree_split_leaf(child, &sibling_key);
    btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
    sibling->m_leaf.m_next = child->m_leaf.m_next;
    child->m_leaf.m_next = sibling;
    if(key >= sibling_key) 
    {
      child = sibling;
    }
  }
  return child;
}


BTInsert 
btree_insert(BTRoot* root, 
             BTNode* node, 
             uint32_t key) 
{
  if(node->m_type == BTNodeType::E_INTERNAL) 
  {
    uint32_t child_idx = btree_next_internal(node, key);
    BTNode* child = node->m_internal.m_children[child_idx];
    if(child == nullptr) 
    {
      if( node->m_internal.m_nchildren == 0 || node->m_internal.m_children[0]->m_type == BTNodeType::E_LEAF)  
      {
        node->m_internal.m_children[child_idx] = btree_create_leaf();
        child = node->m_internal.m_children[child_idx];
      } 
      else 
      {
        node->m_internal.m_children[child_idx] = btree_create_internal();
        child = node->m_internal.m_children[child_idx];
      }
      if(child_idx > 0) 
      {
        node->m_internal.m_keys[child_idx-1] = key;
      }
      node->m_internal.m_nchildren++;
    }

    child = btree_split_child_full(node, child_idx, key);
    return btree_insert(root, child, key);
  } 
  else 
  { 
    uint32_t pos = btree_next_leaf(node, key);
    if(node->m_leaf.m_keys[pos] != key ||
       node->m_leaf.m_leafs[pos] == nullptr)
    {
      btree_shift_insert_leaf(root, node, pos, key);
      return BTInsert{true, &node->m_leaf.m_leafs[pos]};
    }
    return BTInsert{false, &node->m_leaf.m_leafs[pos]};
  }
  return BTInsert{false, nullptr};
}

BTInsert 
btree_insert_root(BTRoot* bt_root, 
                  uint32_t key) 
{
  BTNode* root = bt_root->p_root;
  assert(root->m_type == BTNodeType::E_INTERNAL);
  if(root->m_internal.m_nchildren == BTREE_INTERNAL_MAX_ARITY) 
  {
      uint32_t sibling_key;
      BTNode* sibling = btree_split_internal(root, &sibling_key);
      BTNode* new_root = btree_create_internal();
      new_root->m_internal.m_children[0] = root;
      new_root->m_internal.m_children[1] = sibling;
      new_root->m_internal.m_keys[0] = sibling_key;
      new_root->m_internal.m_nchildren = 2;
      bt_root->p_root = new_root;
      root = new_root;
  }
  uint32_t child_idx = btree_next_internal(root, key);
  BTNode* child = root->m_internal.m_children[child_idx];
  if(child == nullptr) 
  {
    if( root->m_internal.m_nchildren == 0 || 
        root->m_internal.m_children[0]->m_type == BTNodeType::E_LEAF)  
    {
      root->m_internal.m_children[child_idx] = btree_create_leaf();
      child = root->m_internal.m_children[child_idx];
    } 
    else 
    {
      root->m_internal.m_children[child_idx] = btree_create_internal();
      child = root->m_internal.m_children[child_idx];
    }
    if(child_idx > 0) 
    {
      root->m_internal.m_keys[child_idx-1] = key;
    }
    root->m_internal.m_nchildren++;
  }
  child = btree_split_child_full(root, child_idx, key);
  return btree_insert(bt_root,child, key);
}

void 
btree_remove_shift_internal(BTNode* node, uint32_t idx) {
  BTNode* old = node->m_internal.m_children[idx];
  for (uint32_t i = idx; i < BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  { 
    node->m_internal.m_children[i] = node->m_internal.m_children[i+1];
  }
  node->m_internal.m_children[node->m_internal.m_nchildren - 1] = nullptr;

  uint32_t start = idx==0 ? 0 : idx - 1;
  for (uint32_t i = start; i < BTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i+1];
  }

  node->m_internal.m_nchildren--;
  btree_destroy_node(old);
}

void* 
btree_remove_shift_leaf(BTNode* node, uint32_t idx) 
{
  void* value = node->m_leaf.m_leafs[idx];
  node->m_leaf.m_leafs[idx] = nullptr;
  for (uint32_t i = idx; i < BTREE_LEAF_MAX_ARITY-1; ++i) 
  { 
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i+1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i+1];
  }
  node->m_leaf.m_leafs[node->m_leaf.m_nleafs - 1] = nullptr;
  node->m_leaf.m_nleafs--;
  return value;
}

void 
btree_merge_internal(BTNode* node, 
                     uint32_t idx1, 
                     uint32_t idx2) 
{
  assert(idx2 == idx1+1);
  BTNode* child1 = node->m_internal.m_children[idx1];
  BTNode* child2 = node->m_internal.m_children[idx2];
  uint32_t num_elements1 = child1->m_internal.m_nchildren;
  uint32_t num_elements2 = child2->m_internal.m_nchildren;
  assert(num_elements1 + num_elements2 <= BTREE_INTERNAL_MAX_ARITY);
  for (uint32_t i = 0; i < child2->m_internal.m_nchildren; ++i) 
  {
    child1->m_internal.m_children[child1->m_internal.m_nchildren] = child2->m_internal.m_children[i];
    child1->m_internal.m_nchildren++;
    child2->m_internal.m_children[i] = nullptr;
  }
  if(child2->m_internal.m_nchildren > 1) 
  {
    uint32_t num_keys1 = num_elements1-1;
    uint32_t num_keys2 = num_elements2-1;
    child1->m_internal.m_keys[num_keys1] = node->m_internal.m_keys[idx2-1];
    for (uint32_t i = 0; i < num_keys2; ++i) 
    {
      child1->m_internal.m_keys[num_keys1 + i + 1] = child2->m_internal.m_keys[i];
    }
  }
  btree_remove_shift_internal(node, idx2);
}

void 
btree_merge_leaf(BTNode* node, 
                 uint32_t idx1, 
                 uint32_t idx2) 
{
  assert(idx2 == idx1+1);
  BTNode* child1 = node->m_internal.m_children[idx1];
  BTNode* child2 = node->m_internal.m_children[idx2];
  for (uint32_t i = 0; i < child2->m_leaf.m_nleafs; ++i) {
    child1->m_leaf.m_leafs[child1->m_leaf.m_nleafs] = child2->m_leaf.m_leafs[i];
    child1->m_leaf.m_keys[child1->m_leaf.m_nleafs] = child2->m_leaf.m_keys[i];
    child1->m_leaf.m_nleafs++;
    child2->m_leaf.m_leafs[i] = nullptr;
  }
  child1->m_leaf.m_next = child2->m_leaf.m_next;
  btree_remove_shift_internal(node, idx2);
}

void* 
btree_remove(BTNode* node, 
             uint32_t key, 
             bool* min_changed, 
             uint32_t* new_min) 
{
  *min_changed = false;
  if(node->m_type == BTNodeType::E_INTERNAL) 
  {
    uint32_t child_idx = btree_next_internal(node, key);
    BTNode* child = node->m_internal.m_children[child_idx];
    void* removed = btree_remove(child, key, min_changed, new_min);
    if(child_idx > 0 && *min_changed) 
    {
      node->m_internal.m_keys[child_idx - 1] = *new_min;
      *min_changed = false;
    }
    if((child->m_type == BTNodeType::E_INTERNAL && child->m_internal.m_nchildren == 0)
       || (child->m_type == BTNodeType::E_LEAF && child->m_leaf.m_nleafs == 0) ) 
    {
      if(child_idx == 0) 
      {
        *new_min = node->m_internal.m_keys[0];
        *min_changed = true;
      }
      btree_remove_shift_internal(node, child_idx);
    } 
    if(child_idx < BTREE_INTERNAL_MAX_ARITY - 1 && 
       node->m_internal.m_children[child_idx+1] != nullptr) 
    {
      BTNode* child1 = node->m_internal.m_children[child_idx];
      BTNode* child2 = node->m_internal.m_children[child_idx+1];
      if(child1->m_type == BTNodeType::E_INTERNAL) 
      {
        if(child1->m_internal.m_nchildren + child2->m_internal.m_nchildren <= BTREE_INTERNAL_MAX_ARITY) 
        {
          btree_merge_internal(node, child_idx, child_idx+1);
        }
      } 
      else 
      {
        if(child1->m_leaf.m_nleafs + child2->m_leaf.m_nleafs <= BTREE_LEAF_MAX_ARITY) 
        {
          btree_merge_leaf(node, child_idx, child_idx+1);
        }
      }
    }
    return removed;
  } 
  else 
  { //LEAF
    uint32_t child_idx = btree_next_leaf(node, key);
    if(node->m_leaf.m_leafs[child_idx] == nullptr || 
       node->m_leaf.m_keys[child_idx] != key) 
    {
      return nullptr;
    }
    void* value = btree_remove_shift_leaf(node, child_idx);
    if(child_idx == 0) 
    {
      *min_changed = true;
      *new_min = node->m_leaf.m_keys[child_idx];
    }
    return value;
  }
  return nullptr;;
}

void* 
btree_remove_root(BTRoot* bt_root, 
                   uint32_t key) 
{
  bool min_changed;
  uint32_t new_min;
  BTNode* root = bt_root->p_root;
  void* value = btree_remove(root, key, &min_changed, &new_min);
  if(root->m_internal.m_nchildren == 1 && 
     root->m_internal.m_children[0]->m_type == BTNodeType::E_INTERNAL) 
  {
    bt_root->p_root = root->m_internal.m_children[0];
    root->m_internal.m_nchildren=0;
    root->m_internal.m_children[0]=nullptr;
    btree_destroy_node(root);
  }
  return value; 
}

} /* furious */ 
