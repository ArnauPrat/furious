
#include "btree_impl.h"
#include "../memory/memory.h"
#include "../platform.h"
#include <string.h>

namespace furious 
{

btree_iter_t
btree_iter_create(btree_t* btree)
{
  btree_iter_t iter;
  iter.p_root = btree;
  iter.m_index = 0;
  iter.p_leaf = btree->p_root;
  while(iter.p_leaf != nullptr && 
        iter.p_leaf->m_type == btree_node_type_t::E_INTERNAL) 
  {
    iter.p_leaf = iter.p_leaf->m_internal.m_children[0];
  }
  return iter;
}

void
btree_iter_destroy(btree_iter_t* iter)
{
}

bool 
btree_iter_has_next(btree_iter_t* iter)
{
  if(iter->p_leaf == nullptr) 
  {
    return false;
  }
  if (iter->m_index < iter->p_leaf->m_leaf.m_nleafs) 
  {
    return true;
  } 
  btree_node_t* sibling = iter->p_leaf->m_leaf.m_next;
  if(sibling == nullptr) 
  {
    return false;
  }
  return sibling->m_leaf.m_nleafs > 0;
}

btree_entry_t
btree_iter_next(btree_iter_t* iter)
{
  if(iter->p_leaf == nullptr || iter->p_leaf->m_leaf.m_nleafs == 0) 
  {
    return btree_entry_t{0xffffffff,nullptr};
  }
  uint32_t id = iter->p_leaf->m_leaf.m_keys[iter->m_index];
  void* value = iter->p_leaf->m_leaf.m_leafs[iter->m_index];
  iter->m_index+=1;
  if(iter->m_index >= iter->p_leaf->m_leaf.m_nleafs) 
  {
    iter->m_index = 0;
    iter->p_leaf = iter->p_leaf->m_leaf.m_next;
  }
  return btree_entry_t{id, value};
}

btree_node_t* 
btree_create_internal(btree_t* btree) 
{
  btree_node_t* node = (btree_node_t*)mem_alloc(&btree->m_allocator, 
                                                64, 
                                                sizeof(btree_node_t), 
                                                -1); 
  memset(node,0,sizeof(btree_node_t));
  node->m_type = btree_node_type_t::E_INTERNAL;
  return node;
}

btree_node_t* 
btree_create_leaf(btree_t* btree) 
{
  btree_node_t* node = (btree_node_t*)mem_alloc(&btree->m_allocator, 
                                                64, 
                                                sizeof(btree_node_t), 
                                                -1); 
  memset(node,0,sizeof(btree_node_t));
  node->m_type = btree_node_type_t::E_LEAF;
  return node;
}

btree_t
btree_create(mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(((allocator == nullptr) ||
                 (allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr)) &&
                 "Provided allocator is ill-formed.")
  btree_t btree;
  if(allocator != nullptr)
  {
    btree.m_allocator = *allocator; 
  }
  else
  {
    btree.m_allocator = global_mem_allocator;
  }
  btree.p_root = btree_create_internal(&btree);
  return btree;
}

void
btree_destroy(btree_t* root)
{
  if(root->p_root != nullptr)
  {
    btree_destroy_node(root, 
                       root->p_root);
    root->p_root = nullptr;
  }
}

void 
btree_destroy_node(btree_t* btree, 
                   btree_node_t* node) 
{
  if (node->m_type == btree_node_type_t::E_INTERNAL) 
  {
    for (uint32_t i = 0; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY; ++i) 
    {
      if (node->m_internal.m_children[i] != nullptr) 
      {
        btree_destroy_node(btree, 
                           node->m_internal.m_children[i]);
        node->m_internal.m_children[i] = nullptr;
      }
    }
  } 
  else
  {
    for (uint32_t i = 0; i < FURIOUS_BTREE_LEAF_MAX_ARITY; ++i) 
    {
      if (node->m_leaf.m_leafs[i] != nullptr) 
      {
        node->m_leaf.m_leafs[i] = nullptr;
      }
    }
  }
  mem_free(&btree->m_allocator, 
           node);
}

inline uint32_t
__btree_next_internal(btree_node_t* node,
                      uint32_t key)
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  uint32_t i = 0;
  for (; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY-1 && 
       node->m_internal.m_children[i+1] != nullptr && 
       key >= node->m_internal.m_keys[i]; 
       ++i);
  return i;
}

uint32_t 
btree_next_internal(btree_node_t* node, 
                    uint32_t key) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  return __btree_next_internal(node, key);
}

inline uint32_t 
__btree_next_leaf(btree_node_t* node, 
                  uint32_t key) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  /*uint32_t i = 0;
  for (; i < FURIOUS_BTREE_LEAF_MAX_ARITY && 
       node->m_leaf.m_leafs[i] != nullptr && 
       key > node->m_leaf.m_keys[i]; 
       ++i);
  return i;
  */
  int32_t left = 0;
  int32_t right = node->m_leaf.m_nleafs;
  while(left < right)
  {
    int32_t mid = (right + left) / 2;
    if(node->m_leaf.m_keys[mid] < key)
    {
      left = mid+1;
    }
    else
    {
      right = mid;
    }
  }
  return left;
}


uint32_t 
btree_next_leaf(btree_node_t* node, 
                uint32_t key) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  return __btree_next_leaf(node, key);
}

void*
btree_get_node(btree_node_t* node, 
               uint32_t ekey) 
{
  if(node->m_type == btree_node_type_t::E_INTERNAL) 
  {
    uint32_t child_idx = __btree_next_internal(node, ekey);
    if(child_idx < FURIOUS_BTREE_INTERNAL_MAX_ARITY)
    {
      btree_node_t* child = node->m_internal.m_children[child_idx];
      if(child != nullptr ) 
      { 
        // if we found such a children, then proceed. If nullptr, means the node is empty
        return btree_get_node(child, ekey);
      }
    }
  } 
  else 
  { // LEAF NODE
    uint32_t i = __btree_next_leaf(node, ekey);
    if(i < FURIOUS_BTREE_LEAF_MAX_ARITY &&
       node->m_leaf.m_leafs[i] != nullptr && 
       node->m_leaf.m_keys[i] == ekey)
    {
      return node->m_leaf.m_leafs[i];
    }
  }
  return nullptr;
}

void* 
btree_get(btree_t* root, 
          uint32_t ekey)
{
  void* value = btree_get_node(root->p_root, ekey);
  if(value != nullptr)
  {
    return value;
  }
  return nullptr;
}


btree_node_t* 
btree_split_internal(btree_t* btree, 
                     FURIOUS_RESTRICT(btree_node_t*) node,
                     uint32_t* sibling_key) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  FURIOUS_RESTRICT(btree_node_t*) sibling = btree_create_internal(btree);
  uint32_t start = FURIOUS_BTREE_INTERNAL_MIN_ARITY;
  *sibling_key = node->m_internal.m_keys[start-1];
  node->m_internal.m_keys[start-1] = 0;
  for(uint32_t i = start; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    sibling->m_internal.m_children[i-start] = node->m_internal.m_children[i];
    sibling->m_internal.m_nchildren++;
    node->m_internal.m_children[i] = nullptr;
    node->m_internal.m_nchildren--;
  }
  for(uint32_t i = start; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY-1; ++i ) 
  {
    sibling->m_internal.m_keys[i-start] = node->m_internal.m_keys[i];
    node->m_internal.m_keys[i] = 0;
  }
  return sibling;
}

btree_node_t* 
btree_split_leaf(btree_t* btree, 
                 FURIOUS_RESTRICT(btree_node_t*) node, 
                 uint32_t* sibling_key) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  FURIOUS_RESTRICT(btree_node_t*) sibling = btree_create_leaf(btree);
  uint32_t start = FURIOUS_BTREE_LEAF_MIN_ARITY;
  *sibling_key = node->m_leaf.m_keys[start];
  for(uint32_t i = start; i < FURIOUS_BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    sibling->m_leaf.m_leafs[i-start] = node->m_leaf.m_leafs[i];
    sibling->m_leaf.m_keys[i-start] = node->m_leaf.m_keys[i];
    sibling->m_leaf.m_nleafs++;
    node->m_leaf.m_leafs[i] = nullptr;
    node->m_leaf.m_keys[i] = 0;
    node->m_leaf.m_nleafs--;
  }
  return sibling;
}

void 
btree_shift_insert_internal(FURIOUS_RESTRICT(btree_node_t*) node, 
                            uint32_t idx, 
                            FURIOUS_RESTRICT(btree_node_t*) child, 
                            uint32_t key ) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  FURIOUS_ASSERT(idx > 0 && "Index must be greater than 0");
  for(uint32_t i = FURIOUS_BTREE_INTERNAL_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_internal.m_children[i] = node->m_internal.m_children[i-1];
  }

  for(uint32_t i = FURIOUS_BTREE_INTERNAL_MAX_ARITY-2; i > idx-1; --i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i-1];
  }
  node->m_internal.m_children[idx] = child;
  node->m_internal.m_keys[idx-1] = key;
  node->m_internal.m_nchildren++;
}

void 
btree_shift_insert_leaf(btree_t* root,
                        btree_node_t* node, 
                        uint32_t idx, 
                        uint32_t key ) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  for(uint32_t i = FURIOUS_BTREE_LEAF_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i-1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i-1];
  }

  node->m_leaf.m_leafs[idx] = nullptr;
  node->m_leaf.m_keys[idx] = key;
  node->m_leaf.m_nleafs++;
}


btree_node_t* 
btree_split_child_full(btree_t* btree, 
                       FURIOUS_RESTRICT(btree_node_t*) node, 
                       uint32_t child_idx, 
                       uint32_t key)
{
  FURIOUS_RESTRICT(btree_node_t*) child = node->m_internal.m_children[child_idx];
  if(child->m_type == btree_node_type_t::E_INTERNAL && child->m_internal.m_nchildren == FURIOUS_BTREE_INTERNAL_MAX_ARITY) 
  {
    uint32_t sibling_key;
    FURIOUS_RESTRICT(btree_node_t*) sibling = btree_split_internal(btree, 
                                                 child, 
                                                 &sibling_key);
    btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
    if(key >= sibling_key) 
    {
      child = sibling;
    }
  } 
  else if( child->m_type == btree_node_type_t::E_LEAF && child->m_leaf.m_nleafs == FURIOUS_BTREE_LEAF_MAX_ARITY )
  {
    uint32_t sibling_key;
    //if(key <= child->m_leaf.m_keys[FURIOUS_BTREE_LEAF_MAX_ARITY-1] )
    {
      FURIOUS_RESTRICT(btree_node_t*) sibling = btree_split_leaf(btree, 
                                               child, 
                                               &sibling_key);
      btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
      sibling->m_leaf.m_next = child->m_leaf.m_next;
      child->m_leaf.m_next = sibling;
      if(key >= sibling_key) 
      {
        child = sibling;
      }
    }
    /*else
    {
      btree_node_t* sibling = btree_create_leaf();
      node->m_internal.m_children[child_idx+1] = sibling;
      node->m_internal.m_keys[child_idx] = key;
      node->m_internal.m_nchildren++;
      sibling->m_leaf.m_next = child->m_leaf.m_next;
      child->m_leaf.m_next = sibling;
      child = sibling;
    }*/
  }
  return child;
}


btree_insert_t 
btree_insert_node(btree_t* btree, 
                  FURIOUS_RESTRICT(btree_node_t*) node, 
                  uint32_t key, 
                  void* ptr) 
{
  if(node->m_type == btree_node_type_t::E_INTERNAL) 
  {
    uint32_t child_idx = btree_next_internal(node, key);
    FURIOUS_RESTRICT(btree_node_t*) child = node->m_internal.m_children[child_idx];
    if(child == nullptr) 
    {
      if( node->m_internal.m_nchildren == 0 || node->m_internal.m_children[0]->m_type == btree_node_type_t::E_LEAF)  
      {
        node->m_internal.m_children[child_idx] = btree_create_leaf(btree);
        child = node->m_internal.m_children[child_idx];
      } 
      else 
      {
        node->m_internal.m_children[child_idx] = btree_create_internal(btree);
        child = node->m_internal.m_children[child_idx];
      }
      if(child_idx > 0) 
      {
        node->m_internal.m_keys[child_idx-1] = key;
      }
      node->m_internal.m_nchildren++;
    }

    child = btree_split_child_full(btree, 
                                   node, 
                                   child_idx, 
                                   key);
    return btree_insert_node(btree, child, key, ptr);
  } 
  else 
  { 
    uint32_t pos = btree_next_leaf(node, key);
    if(node->m_leaf.m_keys[pos] != key ||
       node->m_leaf.m_leafs[pos] == nullptr)
    {
      btree_shift_insert_leaf(btree, 
                              node, 
                              pos, 
                              key);
      node->m_leaf.m_leafs[pos] = ptr;
      return btree_insert_t{true, &node->m_leaf.m_leafs[pos]};
    }
    return btree_insert_t{false, &node->m_leaf.m_leafs[pos]};
  }
  return btree_insert_t{false, nullptr};
}

btree_insert_t 
btree_insert(btree_t* btree, 
             uint32_t key, 
             void* ptr) 
{
  btree_node_t* root = btree->p_root;
  FURIOUS_ASSERT(root->m_type == btree_node_type_t::E_INTERNAL);
  if(root->m_internal.m_nchildren == FURIOUS_BTREE_INTERNAL_MAX_ARITY) 
  {
    uint32_t sibling_key;
    FURIOUS_RESTRICT(btree_node_t*) sibling = btree_split_internal(btree, 
                                                 root, 
                                                 &sibling_key);
    FURIOUS_RESTRICT(btree_node_t*) new_root = btree_create_internal(btree);
    new_root->m_internal.m_children[0] = root;
    new_root->m_internal.m_children[1] = sibling;
    new_root->m_internal.m_keys[0] = sibling_key;
    new_root->m_internal.m_nchildren = 2;
    btree->p_root = new_root;
    root = new_root;
  }
  uint32_t child_idx = btree_next_internal(root, key);
  FURIOUS_RESTRICT(btree_node_t*) child = root->m_internal.m_children[child_idx];
  if(child == nullptr) 
  {
    if( root->m_internal.m_nchildren == 0 || 
        root->m_internal.m_children[0]->m_type == btree_node_type_t::E_LEAF)  
    {
      root->m_internal.m_children[child_idx] = btree_create_leaf(btree);
      child = root->m_internal.m_children[child_idx];
    } 
    else 
    {
      root->m_internal.m_children[child_idx] = btree_create_internal(btree);
      child = root->m_internal.m_children[child_idx];
    }
    if(child_idx > 0) 
    {
      root->m_internal.m_keys[child_idx-1] = key;
    }
    root->m_internal.m_nchildren++;
  }
  child = btree_split_child_full(btree,
                                 root, 
                                 child_idx, 
                                 key);
  return btree_insert_node(btree,
                           child, 
                           key, 
                           ptr);
}

void 
btree_remove_shift_internal(btree_t* btree, 
                            FURIOUS_RESTRICT(btree_node_t*) node, 
                            uint32_t idx) {
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  FURIOUS_RESTRICT(btree_node_t*) old = node->m_internal.m_children[idx];
  for (uint32_t i = idx; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  { 
    node->m_internal.m_children[i] = node->m_internal.m_children[i+1];
  }
  node->m_internal.m_children[node->m_internal.m_nchildren - 1] = nullptr;

  uint32_t start = idx==0 ? 0 : idx - 1;
  for (uint32_t i = start; i < FURIOUS_BTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i+1];
  }

  node->m_internal.m_nchildren--;
  btree_destroy_node(btree, 
                     old);
}

void* 
btree_remove_shift_leaf(btree_node_t* node, uint32_t idx) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  void* value = node->m_leaf.m_leafs[idx];
  node->m_leaf.m_leafs[idx] = nullptr;
  for (uint32_t i = idx; i < FURIOUS_BTREE_LEAF_MAX_ARITY-1; ++i) 
  { 
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i+1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i+1];
  }
  node->m_leaf.m_leafs[node->m_leaf.m_nleafs - 1] = nullptr;
  node->m_leaf.m_nleafs--;
  return value;
}

void 
btree_merge_internal(btree_t* btree, 
                     FURIOUS_RESTRICT(btree_node_t*) node, 
                     uint32_t idx1, 
                     uint32_t idx2) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  FURIOUS_RESTRICT(btree_node_t*) child1 = node->m_internal.m_children[idx1];
  FURIOUS_RESTRICT(btree_node_t*) child2 = node->m_internal.m_children[idx2];
  uint32_t num_elements1 = child1->m_internal.m_nchildren;
  uint32_t num_elements2 = child2->m_internal.m_nchildren;
  assert(num_elements1 + num_elements2 <= FURIOUS_BTREE_INTERNAL_MAX_ARITY);
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
  btree_remove_shift_internal(btree, node, idx2);
}

void 
btree_merge_leaf(btree_t* btree, 
                 FURIOUS_RESTRICT(btree_node_t*) node, 
                 uint32_t idx1, 
                 uint32_t idx2) 
{
  FURIOUS_ASSERT(node->m_type == btree_node_type_t::E_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  FURIOUS_RESTRICT(btree_node_t*) child1 = node->m_internal.m_children[idx1];
  FURIOUS_RESTRICT(btree_node_t*) child2 = node->m_internal.m_children[idx2];
  FURIOUS_ASSERT(child1->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  FURIOUS_ASSERT(child2->m_type == btree_node_type_t::E_LEAF && "Node must be a leaf node");
  for (uint32_t i = 0; i < child2->m_leaf.m_nleafs; ++i) {
    child1->m_leaf.m_leafs[child1->m_leaf.m_nleafs] = child2->m_leaf.m_leafs[i];
    child1->m_leaf.m_keys[child1->m_leaf.m_nleafs] = child2->m_leaf.m_keys[i];
    child1->m_leaf.m_nleafs++;
    child2->m_leaf.m_leafs[i] = nullptr;
  }
  child1->m_leaf.m_next = child2->m_leaf.m_next;
  btree_remove_shift_internal(btree, 
                              node, 
                              idx2);
}

void* 
btree_remove_node(btree_t* btree, 
                  FURIOUS_RESTRICT(btree_node_t*) node, 
                  uint32_t key, 
                  bool* min_changed, 
                  uint32_t* new_min) 
{
  *min_changed = false;
  if(node->m_type == btree_node_type_t::E_INTERNAL) 
  {
    uint32_t child_idx = btree_next_internal(node, key);
    FURIOUS_RESTRICT(btree_node_t*) child = node->m_internal.m_children[child_idx];
    void* removed = btree_remove_node(btree, 
                                      child, 
                                      key, 
                                      min_changed, 
                                      new_min);
    if(child_idx > 0 && *min_changed) 
    {
      node->m_internal.m_keys[child_idx - 1] = *new_min;
      *min_changed = false;
    }
    if((child->m_type == btree_node_type_t::E_INTERNAL && child->m_internal.m_nchildren == 0)
       || (child->m_type == btree_node_type_t::E_LEAF && child->m_leaf.m_nleafs == 0) ) 
    {
      if(child_idx == 0) 
      {
        *new_min = node->m_internal.m_keys[0];
        *min_changed = true;
      }
      btree_remove_shift_internal(btree, 
                                  node, 
                                  child_idx);
    } 
    if(child_idx < FURIOUS_BTREE_INTERNAL_MAX_ARITY - 1 && 
       node->m_internal.m_children[child_idx+1] != nullptr) 
    {
      FURIOUS_RESTRICT(btree_node_t*) child1 = node->m_internal.m_children[child_idx];
      FURIOUS_RESTRICT(btree_node_t*) child2 = node->m_internal.m_children[child_idx+1];
      if(child1->m_type == btree_node_type_t::E_INTERNAL) 
      {
        if(child1->m_internal.m_nchildren + child2->m_internal.m_nchildren <= FURIOUS_BTREE_INTERNAL_MAX_ARITY) 
        {
          btree_merge_internal(btree, 
                               node, 
                               child_idx, 
                               child_idx+1);
        }
      } 
      else 
      {
        if(child1->m_leaf.m_nleafs + child2->m_leaf.m_nleafs <= FURIOUS_BTREE_LEAF_MAX_ARITY) 
        {
          btree_merge_leaf(btree, 
                           node, 
                           child_idx, 
                           child_idx+1);
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
btree_remove(btree_t* btree, 
             uint32_t key) 
{
  bool min_changed;
  uint32_t new_min;
  btree_node_t* root = btree->p_root;
  void* value = btree_remove_node(btree, 
                                  root, 
                                  key, 
                                  &min_changed, 
                                  &new_min);
  if(root->m_internal.m_nchildren == 1 && 
     root->m_internal.m_children[0]->m_type == btree_node_type_t::E_INTERNAL) 
  {
    btree->p_root = root->m_internal.m_children[0];
    root->m_internal.m_nchildren=0;
    root->m_internal.m_children[0]=nullptr;
    btree_destroy_node(btree, 
                       root);
  }
  return value; 
}

} /* furious */ 
