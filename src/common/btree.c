
#include "btree.h"
#include "memory/pool_allocator.h"

#include <string.h>

void
fdb_btree_iter_init(struct fdb_btree_iter_t* iter, 
                    const struct fdb_btree_t* btree)
{
  iter->p_root = btree;
  iter->m_index = 0;
  iter->p_leaf = btree->p_root;
  while(iter->p_leaf != NULL && 
        iter->p_leaf->m_type == E_INTERNAL) 
  {
    iter->p_leaf = iter->p_leaf->m_internal.m_children[0];
  }
}

void
fdb_btree_iter_release(struct fdb_btree_iter_t* iter)
{
}

bool 
fdb_btree_iter_has_next(struct fdb_btree_iter_t* iter)
{
  if(iter->p_leaf == NULL) 
  {
    return false;
  }
  if (iter->m_index < iter->p_leaf->m_leaf.m_nleafs) 
  {
    return true;
  } 
  struct fdb_btree_node_t* sibling = iter->p_leaf->m_leaf.m_next;
  if(sibling == NULL) 
  {
    return false;
  }
  return sibling->m_leaf.m_nleafs > 0;
}

struct fdb_btree_entry_t
fdb_btree_iter_next(struct fdb_btree_iter_t* iter)
{
  if(iter->p_leaf == NULL || iter->p_leaf->m_leaf.m_nleafs == 0) 
  {
    struct fdb_btree_entry_t entry = {0xffffffff,NULL};
    return entry;
  }
  uint32_t id = iter->p_leaf->m_leaf.m_keys[iter->m_index];
  void* value = iter->p_leaf->m_leaf.m_leafs[iter->m_index];
  iter->m_index+=1;
  if(iter->m_index >= iter->p_leaf->m_leaf.m_nleafs) 
  {
    iter->m_index = 0;
    iter->p_leaf = iter->p_leaf->m_leaf.m_next;
  }
  struct fdb_btree_entry_t entry = {id, value};
  return entry;
}

struct fdb_btree_node_t* 
fdb_btree_create_internal(struct fdb_btree_t* btree) 
{
  struct fdb_btree_node_t* node = (struct fdb_btree_node_t*)fdb_pool_alloc_alloc(&btree->p_factory->m_node_allocator, 
                                                                   FDB_BTREE_NODE_ALIGNMENT, 
                                                                   sizeof(struct fdb_btree_node_t), 
                                                                   -1); 
  memset(node,0,sizeof(struct fdb_btree_node_t));
  node->m_type = E_INTERNAL;
  return node;
}

struct fdb_btree_node_t* 
fdb_btree_create_leaf(struct fdb_btree_t* btree) 
{
  struct fdb_btree_node_t* node = (struct fdb_btree_node_t*)fdb_pool_alloc_alloc(&btree->p_factory->m_node_allocator, 
                                                                   FDB_BTREE_NODE_ALIGNMENT, 
                                                                   sizeof(struct fdb_btree_node_t), 
                                                                   -1); 
  memset(node,0,sizeof(struct fdb_btree_node_t));
  node->m_type = E_LEAF;
  return node;
}

void
fdb_btree_factory_init(struct fdb_btree_factory_t* factory, 
                       struct fdb_mem_allocator_t* allocator)
{

  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  *factory = (struct fdb_btree_factory_t){};
  fdb_pool_alloc_init(&factory->m_node_allocator, 
                      FDB_BTREE_ALIGNMENT, 
                      sizeof(struct fdb_btree_node_t),
                      FDB_BTREE_PAGE_SIZE,
                      allocator);

}

void
fdb_btree_factory_release(struct fdb_btree_factory_t* factory)
{
  fdb_pool_alloc_release(&factory->m_node_allocator);
}

void
fdb_btree_init(struct fdb_btree_t* btree, 
               struct fdb_btree_factory_t* factory)
{
  btree->p_factory = factory;
  btree->m_size = 0;
  btree->p_root = fdb_btree_create_internal(btree);
}

void
fdb_btree_release(struct fdb_btree_t* root)
{
  fdb_btree_destroy_node(root, 
                         root->p_root);
  root->p_root = NULL;
}

void
fdb_btree_clear(struct fdb_btree_t* btree)
{
  fdb_btree_destroy_node(btree, 
                         btree->p_root);
  btree->p_root = fdb_btree_create_internal(btree);
  btree->m_size = 0;
}

void 
fdb_btree_destroy_node(struct fdb_btree_t* btree, 
                       struct fdb_btree_node_t* node) 
{
  if (node->m_type == E_INTERNAL) 
  {
    for (uint32_t i = 0; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
    {
      if (node->m_internal.m_children[i] != NULL) 
      {
        fdb_btree_destroy_node(btree, 
                               node->m_internal.m_children[i]);
        node->m_internal.m_children[i] = NULL;
      }
    }
  } 
  else
  {
    for (uint32_t i = 0; i < FDB_BTREE_LEAF_MAX_ARITY; ++i) 
    {
      if (node->m_leaf.m_leafs[i] != NULL) 
      {
        node->m_leaf.m_leafs[i] = NULL;
      }
    }
  }
  fdb_pool_alloc_free(&btree->p_factory->m_node_allocator, 
                      node);
}

uint32_t
__fdb_btree_next_internal(const struct fdb_btree_node_t* node,
                          uint32_t key)
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  uint32_t i = 0;
  for (; i < FDB_BTREE_INTERNAL_MAX_ARITY-1 && 
       node->m_internal.m_children[i+1] != NULL && 
       key >= node->m_internal.m_keys[i]; 
       ++i);
  return i;
}

uint32_t 
fdb_btree_next_internal(const struct fdb_btree_node_t* node, 
                        uint32_t key) 
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  return __fdb_btree_next_internal(node, key);
}

uint32_t 
__fdb_btree_next_leaf(const struct fdb_btree_node_t* node, 
                      uint32_t key) 
{
  FDB_ASSERT(node->m_type == E_LEAF && "Node must be a leaf node");
  uint32_t i = 0;
  for (; i < FDB_BTREE_LEAF_MAX_ARITY && 
       node->m_leaf.m_leafs[i] != NULL && 
       key > node->m_leaf.m_keys[i]; 
       ++i);
  return i;
  /*int32_t left = 0;
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
    */
}


uint32_t 
fdb_btree_next_leaf(const struct fdb_btree_node_t* node, 
                    uint32_t key) 
{
  FDB_ASSERT(node->m_type == E_LEAF && "Node must be a leaf node");
  return __fdb_btree_next_leaf(node, key);
}

void*
fdb_btree_get_node(const struct fdb_btree_node_t* node, 
                   uint32_t ekey) 
{
  if(node->m_type == E_INTERNAL) 
  {
    uint32_t child_idx = __fdb_btree_next_internal(node, ekey);
    if(child_idx < FDB_BTREE_INTERNAL_MAX_ARITY)
    {
      struct fdb_btree_node_t* child = node->m_internal.m_children[child_idx];
      if(child != NULL ) 
      { 
        // if we found such a children, then proceed. If NULL, means the node is empty
        return fdb_btree_get_node(child, ekey);
      }
    }
  } 
  else 
  { // LEAF NODE
    uint32_t i = __fdb_btree_next_leaf(node, ekey);
    if(i < FDB_BTREE_LEAF_MAX_ARITY &&
       node->m_leaf.m_leafs[i] != NULL && 
       node->m_leaf.m_keys[i] == ekey)
    {
      return node->m_leaf.m_leafs[i];
    }
  }
  return NULL;
}

void* 
fdb_btree_get(const struct fdb_btree_t* root, 
              uint32_t ekey)
{
  void* value = fdb_btree_get_node(root->p_root, ekey);
  if(value != NULL)
  {
    return value;
  }
  return NULL;
}


struct fdb_btree_node_t* 
fdb_btree_split_internal(struct fdb_btree_t* btree, 
                         FDB_RESTRICT(struct fdb_btree_node_t*) node,
                         uint32_t* sibling_key) 
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  FDB_RESTRICT(struct fdb_btree_node_t*) sibling = fdb_btree_create_internal(btree);
  uint32_t start = FDB_BTREE_INTERNAL_MIN_ARITY;
  *sibling_key = node->m_internal.m_keys[start-1];
  node->m_internal.m_keys[start-1] = 0;
  for(uint32_t i = start; i < FDB_BTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    sibling->m_internal.m_children[i-start] = node->m_internal.m_children[i];
    sibling->m_internal.m_nchildren++;
    node->m_internal.m_children[i] = NULL;
    node->m_internal.m_nchildren--;
  }
  for(uint32_t i = start; i < FDB_BTREE_INTERNAL_MAX_ARITY-1; ++i ) 
  {
    sibling->m_internal.m_keys[i-start] = node->m_internal.m_keys[i];
    node->m_internal.m_keys[i] = 0;
  }
  return sibling;
}

struct fdb_btree_node_t* 
fdb_btree_split_leaf(struct fdb_btree_t* btree, 
                     FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                     uint32_t* sibling_key) 
{
  FDB_ASSERT(node->m_type == E_LEAF && "Node must be a leaf node");
  FDB_RESTRICT(struct fdb_btree_node_t*) sibling = fdb_btree_create_leaf(btree);
  uint32_t start = FDB_BTREE_LEAF_MIN_ARITY;
  *sibling_key = node->m_leaf.m_keys[start];
  for(uint32_t i = start; i < FDB_BTREE_LEAF_MAX_ARITY; ++i ) 
  {
    sibling->m_leaf.m_leafs[i-start] = node->m_leaf.m_leafs[i];
    sibling->m_leaf.m_keys[i-start] = node->m_leaf.m_keys[i];
    sibling->m_leaf.m_nleafs++;
    node->m_leaf.m_leafs[i] = NULL;
    node->m_leaf.m_keys[i] = 0;
    node->m_leaf.m_nleafs--;
  }
  return sibling;
}

void 
fdb_btree_shift_insert_internal(FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                                uint32_t idx, 
                                FDB_RESTRICT(struct fdb_btree_node_t*) child, 
                                uint32_t key ) 
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  FDB_ASSERT(idx > 0 && "Index must be greater than 0");
  for(uint32_t i = FDB_BTREE_INTERNAL_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_internal.m_children[i] = node->m_internal.m_children[i-1];
  }

  for(uint32_t i = FDB_BTREE_INTERNAL_MAX_ARITY-2; i > idx-1; --i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i-1];
  }
  node->m_internal.m_children[idx] = child;
  node->m_internal.m_keys[idx-1] = key;
  node->m_internal.m_nchildren++;
}

void 
fdb_btree_shift_insert_leaf(struct fdb_btree_t* root,
                            struct fdb_btree_node_t* node, 
                            uint32_t idx, 
                            uint32_t key ) 
{
  FDB_ASSERT(node->m_type == E_LEAF && "Node must be a leaf node");
  for(uint32_t i = FDB_BTREE_LEAF_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i-1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i-1];
  }

  node->m_leaf.m_leafs[idx] = NULL;
  node->m_leaf.m_keys[idx] = key;
  node->m_leaf.m_nleafs++;
}


struct fdb_btree_node_t* 
fdb_btree_split_child_full(struct fdb_btree_t* btree, 
                           FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                           uint32_t child_idx, 
                           uint32_t key)
{
  FDB_RESTRICT(struct fdb_btree_node_t*) child = node->m_internal.m_children[child_idx];
  if(child->m_type == E_INTERNAL && child->m_internal.m_nchildren == FDB_BTREE_INTERNAL_MAX_ARITY) 
  {
    uint32_t sibling_key;
    FDB_RESTRICT(struct fdb_btree_node_t*) sibling = fdb_btree_split_internal(btree, 
                                                                       child, 
                                                                       &sibling_key);
    fdb_btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
    if(key >= sibling_key) 
    {
      child = sibling;
    }
  } 
  else if( child->m_type == E_LEAF && child->m_leaf.m_nleafs == FDB_BTREE_LEAF_MAX_ARITY )
  {
    uint32_t sibling_key;
    //if(key <= child->m_leaf.m_keys[FDB_BTREE_LEAF_MAX_ARITY-1] )
    {
      FDB_RESTRICT(struct fdb_btree_node_t*) sibling = fdb_btree_split_leaf(btree, 
                                                                     child, 
                                                                     &sibling_key);
      fdb_btree_shift_insert_internal(node, child_idx+1, sibling, sibling_key );
      sibling->m_leaf.m_next = child->m_leaf.m_next;
      child->m_leaf.m_next = sibling;
      if(key >= sibling_key) 
      {
        child = sibling;
      }
    }
    /*else
      {
      fdb_btree_node_t* sibling = fdb_btree_create_leaf();
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


struct fdb_btree_insert_t 
fdb_btree_insert_node(struct fdb_btree_t* btree, 
                      FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                      uint32_t key, 
                      void* ptr) 
{
  if(node->m_type == E_INTERNAL) 
  {
    uint32_t child_idx = fdb_btree_next_internal(node, key);
    FDB_RESTRICT(struct fdb_btree_node_t*) child = node->m_internal.m_children[child_idx];
    if(child == NULL) 
    {
      if( node->m_internal.m_nchildren == 0 || node->m_internal.m_children[0]->m_type == E_LEAF)  
      {
        node->m_internal.m_children[child_idx] = fdb_btree_create_leaf(btree);
        child = node->m_internal.m_children[child_idx];
      } 
      else 
      {
        node->m_internal.m_children[child_idx] = fdb_btree_create_internal(btree);
        child = node->m_internal.m_children[child_idx];
      }
      if(child_idx > 0) 
      {
        node->m_internal.m_keys[child_idx-1] = key;
      }
      node->m_internal.m_nchildren++;
    }

    child = fdb_btree_split_child_full(btree, 
                                       node, 
                                       child_idx, 
                                       key);
    return fdb_btree_insert_node(btree, child, key, ptr);
  } 
  else 
  { 
    uint32_t pos = fdb_btree_next_leaf(node, key);
    if(node->m_leaf.m_keys[pos] != key ||
       node->m_leaf.m_leafs[pos] == NULL)
    {
      fdb_btree_shift_insert_leaf(btree, 
                                  node, 
                                  pos, 
                                  key);
      node->m_leaf.m_leafs[pos] = ptr;
      btree->m_size++;
      struct fdb_btree_insert_t insert = {.m_inserted = true, 
                                   .p_place = &node->m_leaf.m_leafs[pos]};
      return insert;
    }
      struct fdb_btree_insert_t insert = {.m_inserted = false, 
                                   .p_place = &node->m_leaf.m_leafs[pos]};
    return insert;
  }

  struct fdb_btree_insert_t insert = {.m_inserted = false, 
                               .p_place = NULL};
  return insert;
}

struct fdb_btree_insert_t 
fdb_btree_insert(struct fdb_btree_t* btree, 
                 uint32_t key, 
                 void* ptr) 
{
  struct fdb_btree_node_t* root = btree->p_root;
  FDB_ASSERT(root->m_type == E_INTERNAL);
  if(root->m_internal.m_nchildren == FDB_BTREE_INTERNAL_MAX_ARITY) 
  {
    uint32_t sibling_key;
    FDB_RESTRICT(struct fdb_btree_node_t*) sibling = fdb_btree_split_internal(btree, 
                                                                       root, 
                                                                       &sibling_key);
    FDB_RESTRICT(struct fdb_btree_node_t*) new_root = fdb_btree_create_internal(btree);
    new_root->m_internal.m_children[0] = root;
    new_root->m_internal.m_children[1] = sibling;
    new_root->m_internal.m_keys[0] = sibling_key;
    new_root->m_internal.m_nchildren = 2;
    btree->p_root = new_root;
    root = new_root;
  }
  uint32_t child_idx = fdb_btree_next_internal(root, key);
  FDB_RESTRICT(struct fdb_btree_node_t*) child = root->m_internal.m_children[child_idx];
  if(child == NULL) 
  {
    if( root->m_internal.m_nchildren == 0 || 
        root->m_internal.m_children[0]->m_type == E_LEAF)  
    {
      root->m_internal.m_children[child_idx] = fdb_btree_create_leaf(btree);
      child = root->m_internal.m_children[child_idx];
    } 
    else 
    {
      root->m_internal.m_children[child_idx] = fdb_btree_create_internal(btree);
      child = root->m_internal.m_children[child_idx];
    }
    if(child_idx > 0) 
    {
      root->m_internal.m_keys[child_idx-1] = key;
    }
    root->m_internal.m_nchildren++;
  }
  child = fdb_btree_split_child_full(btree,
                                     root, 
                                     child_idx, 
                                     key);
  return fdb_btree_insert_node(btree,
                               child, 
                               key, 
                               ptr);
}

void 
fdb_btree_remove_shift_internal(struct fdb_btree_t* btree, 
                                FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                                uint32_t idx) {
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  FDB_RESTRICT(struct fdb_btree_node_t*) old = node->m_internal.m_children[idx];
  for (uint32_t i = idx; i < FDB_BTREE_INTERNAL_MAX_ARITY-1; ++i) 
  { 
    node->m_internal.m_children[i] = node->m_internal.m_children[i+1];
  }
  node->m_internal.m_children[node->m_internal.m_nchildren - 1] = NULL;

  uint32_t start = idx==0 ? 0 : idx - 1;
  for (uint32_t i = start; i < FDB_BTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i+1];
  }

  node->m_internal.m_nchildren--;
  fdb_btree_destroy_node(btree, 
                         old);
}

void* 
fdb_btree_remove_shift_leaf(struct fdb_btree_node_t* node, uint32_t idx) 
{
  FDB_ASSERT(node->m_type == E_LEAF && "Node must be a leaf node");
  void* value = node->m_leaf.m_leafs[idx];
  node->m_leaf.m_leafs[idx] = NULL;
  for (uint32_t i = idx; i < FDB_BTREE_LEAF_MAX_ARITY-1; ++i) 
  { 
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i+1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i+1];
  }
  node->m_leaf.m_leafs[node->m_leaf.m_nleafs - 1] = NULL;
  node->m_leaf.m_nleafs--;
  return value;
}

void 
fdb_btree_merge_internal(struct fdb_btree_t* btree, 
                         FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                         uint32_t idx1, 
                         uint32_t idx2) 
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  FDB_RESTRICT(struct fdb_btree_node_t*) child1 = node->m_internal.m_children[idx1];
  FDB_RESTRICT(struct fdb_btree_node_t*) child2 = node->m_internal.m_children[idx2];
  uint32_t num_elements1 = child1->m_internal.m_nchildren;
  uint32_t num_elements2 = child2->m_internal.m_nchildren;
  assert(num_elements1 + num_elements2 <= FDB_BTREE_INTERNAL_MAX_ARITY);
  for (uint32_t i = 0; i < child2->m_internal.m_nchildren; ++i) 
  {
    child1->m_internal.m_children[child1->m_internal.m_nchildren] = child2->m_internal.m_children[i];
    child1->m_internal.m_nchildren++;
    child2->m_internal.m_children[i] = NULL;
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
  fdb_btree_remove_shift_internal(btree, node, idx2);
}

void 
fdb_btree_merge_leaf(struct fdb_btree_t* btree, 
                     FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                     uint32_t idx1, 
                     uint32_t idx2) 
{
  FDB_ASSERT(node->m_type == E_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  FDB_RESTRICT(struct fdb_btree_node_t*) child1 = node->m_internal.m_children[idx1];
  FDB_RESTRICT(struct fdb_btree_node_t*) child2 = node->m_internal.m_children[idx2];
  FDB_ASSERT(child1->m_type == E_LEAF && "Node must be a leaf node");
  FDB_ASSERT(child2->m_type == E_LEAF && "Node must be a leaf node");
  for (uint32_t i = 0; i < child2->m_leaf.m_nleafs; ++i) {
    child1->m_leaf.m_leafs[child1->m_leaf.m_nleafs] = child2->m_leaf.m_leafs[i];
    child1->m_leaf.m_keys[child1->m_leaf.m_nleafs] = child2->m_leaf.m_keys[i];
    child1->m_leaf.m_nleafs++;
    child2->m_leaf.m_leafs[i] = NULL;
  }
  child1->m_leaf.m_next = child2->m_leaf.m_next;
  fdb_btree_remove_shift_internal(btree, 
                                  node, 
                                  idx2);
}

void* 
fdb_btree_remove_node(struct fdb_btree_t* btree, 
                      FDB_RESTRICT(struct fdb_btree_node_t*) node, 
                      uint32_t key, 
                      bool* min_changed, 
                      uint32_t* new_min) 
{
  *min_changed = false;
  if(node->m_type == E_INTERNAL) 
  {
    uint32_t child_idx = fdb_btree_next_internal(node, key);
    FDB_RESTRICT(struct fdb_btree_node_t*) child = node->m_internal.m_children[child_idx];
    void* removed = fdb_btree_remove_node(btree, 
                                          child, 
                                          key, 
                                          min_changed, 
                                          new_min);
    if(child_idx > 0 && *min_changed) 
    {
      node->m_internal.m_keys[child_idx - 1] = *new_min;
      *min_changed = false;
    }
    if((child->m_type == E_INTERNAL && child->m_internal.m_nchildren == 0)
       || (child->m_type == E_LEAF && child->m_leaf.m_nleafs == 0) ) 
    {
      if(child_idx == 0) 
      {
        *new_min = node->m_internal.m_keys[0];
        *min_changed = true;
      }
      fdb_btree_remove_shift_internal(btree, 
                                      node, 
                                      child_idx);
    } 
    if(child_idx < FDB_BTREE_INTERNAL_MAX_ARITY - 1 && 
       node->m_internal.m_children[child_idx+1] != NULL) 
    {
      FDB_RESTRICT(struct fdb_btree_node_t*) child1 = node->m_internal.m_children[child_idx];
      FDB_RESTRICT(struct fdb_btree_node_t*) child2 = node->m_internal.m_children[child_idx+1];
      if(child1->m_type == E_INTERNAL) 
      {
        if(child1->m_internal.m_nchildren + child2->m_internal.m_nchildren <= FDB_BTREE_INTERNAL_MAX_ARITY) 
        {
          fdb_btree_merge_internal(btree, 
                                   node, 
                                   child_idx, 
                                   child_idx+1);
        }
      } 
      else 
      {
        if(child1->m_leaf.m_nleafs + child2->m_leaf.m_nleafs <= FDB_BTREE_LEAF_MAX_ARITY) 
        {
          fdb_btree_merge_leaf(btree, 
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
    uint32_t child_idx = fdb_btree_next_leaf(node, key);
    if(node->m_leaf.m_leafs[child_idx] == NULL || 
       node->m_leaf.m_keys[child_idx] != key) 
    {
      return NULL;
    }
    btree->m_size -= 1;
    void* value = fdb_btree_remove_shift_leaf(node, child_idx);
    if(child_idx == 0) 
    {
      *min_changed = true;
      *new_min = node->m_leaf.m_keys[child_idx];
    }
    return value;
  }
  return NULL;;
}

void* 
fdb_btree_remove(struct fdb_btree_t* btree, 
                 uint32_t key) 
{
  bool min_changed;
  uint32_t new_min;
  struct fdb_btree_node_t* root = btree->p_root;
  void* value = fdb_btree_remove_node(btree, 
                                      root, 
                                      key, 
                                      &min_changed, 
                                      &new_min);
  if(root->m_internal.m_nchildren == 1 && 
     root->m_internal.m_children[0]->m_type == E_INTERNAL) 
  {
    btree->p_root = root->m_internal.m_children[0];
    root->m_internal.m_nchildren=0;
    root->m_internal.m_children[0]=NULL;
    fdb_btree_destroy_node(btree, 
                           root);
  }
  return value; 
}
