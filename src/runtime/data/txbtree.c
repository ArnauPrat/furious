
#include "txbtree.h"
#include <string.h>



void
fdb_txbtree_factory_init(struct fdb_txbtree_factory_t* factory, 
                         struct fdb_mem_allocator_t* allocator)
{

  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")
  *factory = (struct fdb_txbtree_factory_t){};
  factory->p_btree_allocator = fdb_txpool_alloc_create(FDB_TXBTREE_ALIGNMENT, 
                                                       sizeof(struct fdb_txbtree_impl_t), 
                                                       FDB_TXBTREE_PAGE_SIZE, 
                                                       allocator);

  factory->p_node_allocator = fdb_txpool_alloc_create(FDB_TXBTREE_NODE_ALIGNMENT, 
                                                    sizeof(struct fdb_txbtree_node_t), 
                                                    FDB_TXBTREE_NODE_PAGE_SIZE, 
                                                    allocator);
}

void
fdb_txbtree_factory_release(struct fdb_txbtree_factory_t* factory, 
                            struct fdb_tx_t* tx,
                            struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txpool_alloc_destroy(factory->p_btree_allocator, 
                           tx, 
                           txtctx);
  fdb_txpool_alloc_destroy(factory->p_node_allocator, 
                           tx, 
                           txtctx);
}


void
fdb_txbtree_iter_init(struct fdb_txbtree_iter_t* iter, 
                      struct fdb_txbtree_t* btree,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx)
{
  *iter = (struct fdb_txbtree_iter_t){ 
                                .p_btree = btree, 
                                .m_index = 0, 
                                .p_tx = tx, 
                                .p_txtctx = txtctx
                              };
  struct fdb_txbtree_impl_t* btree_impl = (struct fdb_txbtree_impl_t*)fdb_txpool_alloc_ptr(iter->p_btree->p_factory->p_btree_allocator, 
                                                              tx, 
                                                              txtctx, 
                                                              iter->p_btree->m_impl_ref, 
                                                              false);
  iter->m_leaf = btree_impl->m_root;
  struct fdb_txbtree_node_t* leaf = NULL;
  if(iter->m_leaf.p_main != NULL)
  {
    leaf = (struct fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                     tx, 
                                                     txtctx, 
                                                     iter->m_leaf, 
                                                     false);

  }

  while(leaf != NULL && 
        leaf->m_type == E_TXBTREE_INTERNAL) 
  {
    iter->m_leaf = leaf->m_internal.m_children[0];
    leaf = NULL;
    if(iter->m_leaf.p_main != NULL)
    {
      leaf = (struct fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                              tx, 
                                                              txtctx, 
                                                              iter->m_leaf, 
                                                              false);

    }
  }
}

void
fdb_txbtree_iter_release(struct fdb_txbtree_iter_t* iter)
{
}

bool 
fdb_txbtree_iter_has_next(struct fdb_txbtree_iter_t* iter)
{
  if(iter->m_leaf.p_main == NULL) 
  {
    return false;
  }

  struct fdb_txbtree_node_t* leaf = (struct fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(iter->p_btree->p_factory->p_node_allocator, 
                                                                                     iter->p_tx, 
                                                                                     iter->p_txtctx, 
                                                                                     iter->m_leaf, 
                                                                                     false);
  if (iter->m_index < leaf->m_leaf.m_nleafs) 
  {
    return true;
  } 

  struct fdb_txpool_alloc_ref_t sibling_ref = leaf->m_leaf.m_next;
  if(sibling_ref.p_main == NULL) 
  {
    return false;
  }
  struct fdb_txbtree_node_t* sibling = (struct fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(iter->p_btree->p_factory->p_node_allocator, 
                                                                                        iter->p_tx, 
                                                                                        iter->p_txtctx, 
                                                                                        sibling_ref, 
                                                                                        false);
  return sibling->m_leaf.m_nleafs > 0;
}

struct fdb_txbtree_entry_t
fdb_txbtree_iter_next(struct fdb_txbtree_iter_t* iter)
{
  struct fdb_txbtree_node_t* leaf = (struct fdb_txbtree_node_t*)fdb_txpool_alloc_ptr(iter->p_btree->p_factory->p_node_allocator, 
                                                                                     iter->p_tx, 
                                                                                     iter->p_txtctx, 
                                                                                     iter->m_leaf, 
                                                                                     false);
  if(iter->m_leaf.p_main == NULL || leaf->m_leaf.m_nleafs == 0) 
  {
    struct fdb_txbtree_entry_t entry = {.m_key = 0xffffffff,
                                 .m_value_ref = {NULL}};
    return entry;
  }

  uint32_t id = leaf->m_leaf.m_keys[iter->m_index];
  struct fdb_txpool_alloc_ref_t value_ref = leaf->m_leaf.m_leafs[iter->m_index];
  iter->m_index+=1;
  if(iter->m_index >= leaf->m_leaf.m_nleafs) 
  {
    iter->m_index = 0;
    iter->m_leaf = leaf->m_leaf.m_next;
  }
  struct fdb_txbtree_entry_t entry = {.m_key = id, 
    .m_value_ref = value_ref};
  return entry;
}

struct fdb_txpool_alloc_ref_t
fdb_txbtree_create_internal(struct fdb_txbtree_t* btree, 
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx) 
{
  struct fdb_txpool_alloc_ref_t ref = fdb_txpool_alloc_alloc(btree->p_factory->p_node_allocator, 
                                                             tx, 
                                                             txtctx,
                                                             FDB_TXBTREE_NODE_ALIGNMENT, 
                                                             sizeof(struct fdb_txbtree_node_t), 
                                                             FDB_NO_HINT); 

  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                  tx, 
                                                  txtctx, 
                                                  ref, 
                                                  true);
  memset(node,0,sizeof(struct fdb_txbtree_node_t));
  node->m_type = E_TXBTREE_INTERNAL;
  for(uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i)
  {
   node->m_internal.m_children[i] = (struct fdb_txpool_alloc_ref_t){};
  }
  return ref;
}

struct fdb_txpool_alloc_ref_t
fdb_txbtree_create_leaf(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx) 
{
  struct fdb_txpool_alloc_ref_t ref = fdb_txpool_alloc_alloc(btree->p_factory->p_node_allocator, 
                                                      tx, 
                                                      txtctx,
                                                      FDB_TXBTREE_NODE_ALIGNMENT, 
                                                      sizeof(struct fdb_txbtree_node_t), 
                                                      FDB_NO_HINT); 

  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                  tx, 
                                                  txtctx, 
                                                  ref, 
                                                  true);
  memset(node,0,sizeof(struct fdb_txbtree_node_t));
  node->m_type = E_TXBTREE_LEAF;
  node->m_leaf.m_next = (struct fdb_txpool_alloc_ref_t){};
  return ref;
}

void
fdb_txbtree_init(struct fdb_txbtree_t* btree, 
                 struct fdb_txbtree_factory_t* factory, 
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx)
{
  *btree = (struct fdb_txbtree_t){};
  btree->p_factory = factory;
  btree->m_impl_ref = fdb_txpool_alloc_alloc(factory->p_btree_allocator, 
                                             tx, 
                                             txtctx,
                                             FDB_TXBTREE_ALIGNMENT, 
                                             sizeof(struct fdb_txbtree_impl_t), 
                                             FDB_NO_HINT);

  struct fdb_txbtree_impl_t* btree_impl = fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                      tx, 
                                                      txtctx, 
                                                      btree->m_impl_ref, 
                                                      true);
  btree_impl->m_root = fdb_txbtree_create_internal(btree, tx, txtctx);
}

void
fdb_txbtree_release(struct fdb_txbtree_t* btree, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx)
{

  struct fdb_txbtree_impl_t* btree_impl = fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                               tx, 
                                                               txtctx, 
                                                               btree->m_impl_ref, 
                                                               true);
  fdb_txbtree_destroy_node(btree, 
                           tx, 
                           txtctx,
                           btree_impl->m_root);
  btree_impl->m_root = (struct fdb_txpool_alloc_ref_t){};
}

void
fdb_txbtree_clear(struct fdb_txbtree_t* btree, 
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx)
{
  struct fdb_txbtree_impl_t* btree_impl = fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                      tx, 
                                                      txtctx, 
                                                      btree->m_impl_ref, 
                                                      true);
  fdb_txbtree_destroy_node(btree, 
                           tx, 
                           txtctx,
                           btree_impl->m_root);

  btree_impl->m_root = fdb_txbtree_create_internal(btree, 
                                                   tx, 
                                                   txtctx);
}

void 
fdb_txbtree_destroy_node(struct fdb_txbtree_t* btree, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx,
                         struct fdb_txpool_alloc_ref_t ref) 
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                  tx, 
                                                  txtctx,
                                                  ref, 
                                                  true);
  if (node->m_type == E_TXBTREE_INTERNAL) 
  {
    for (uint32_t i = 0; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
    {
      if (node->m_internal.m_children[i].p_main != NULL) 
      {
        fdb_txbtree_destroy_node(btree, 
                                 tx, 
                                 txtctx,
                                 node->m_internal.m_children[i]);
        node->m_internal.m_children[i] = (struct fdb_txpool_alloc_ref_t){};
      }
    }
  } 
  else
  {
    for (uint32_t i = 0; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i) 
    {
      if (node->m_leaf.m_leafs[i].p_main != NULL)
      {

        node->m_leaf.m_leafs[i] = (struct fdb_txpool_alloc_ref_t){};
      }
    }
  }
  fdb_txpool_alloc_free(btree->p_factory->p_node_allocator, 
                        tx, 
                        txtctx,
                        ref);
}

uint32_t 
fdb_txbtree_next_internal(struct fdb_txbtree_t* btree, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          struct fdb_txpool_alloc_ref_t node_ref, 
                          uint32_t key) 
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         false);
  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  uint32_t i = 0;
  for (; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1 && 
       node->m_internal.m_children[i+1].p_main != NULL && 
       key >= node->m_internal.m_keys[i]; 
       ++i);
  return i;
}


uint32_t 
fdb_txbtree_next_leaf(struct fdb_txbtree_t* btree, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      struct fdb_txpool_alloc_ref_t node_ref, 
                      uint32_t key) 
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         false);
  FDB_ASSERT(node->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  uint32_t i = 0;
  for (; i < FDB_TXBTREE_LEAF_MAX_ARITY && 
       node->m_leaf.m_leafs[i].p_main != NULL && 
       key > node->m_leaf.m_keys[i]; 
       ++i);
  return i;
}

struct fdb_txpool_alloc_ref_t 
fdb_txbtree_get_node(struct fdb_txbtree_t* btree,
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx,
                     struct fdb_txpool_alloc_ref_t node_ref, 
                     uint32_t ekey)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         true);
  if(node->m_type == E_TXBTREE_INTERNAL) 
  {
    uint32_t child_idx = fdb_txbtree_next_internal(btree, 
                                                   tx, 
                                                   txtctx, 
                                                   node_ref,
                                                   ekey);
    if(child_idx < FDB_TXBTREE_INTERNAL_MAX_ARITY)
    {
      if(node->m_internal.m_children[child_idx].p_main != NULL)
      { 
        // if we found such a children, then proceed. If NULL, means the node is empty
        return fdb_txbtree_get_node(btree, 
                                    tx, 
                                    txtctx, 
                                    node->m_internal.m_children[child_idx], 
                                    ekey);
      }
    }
  } 
  else 
  { // LEAF NODE
    uint32_t i = fdb_txbtree_next_leaf(btree, tx, txtctx, node_ref, ekey);
    if(i < FDB_TXBTREE_LEAF_MAX_ARITY &&
       node->m_leaf.m_leafs[i].p_main != NULL && 
       node->m_leaf.m_keys[i] == ekey)
    {
      return node->m_leaf.m_leafs[i];
    }
  }
  struct fdb_txpool_alloc_ref_t ret = (struct fdb_txpool_alloc_ref_t){};
  return ret;
}

struct fdb_txpool_alloc_ref_t 
fdb_txbtree_get(struct fdb_txbtree_t* btree, 
                struct fdb_tx_t* tx, 
                struct fdb_txthread_ctx_t* txtctx,
                uint32_t ekey)
{
  struct fdb_txbtree_impl_t* btree_impl = (struct fdb_txbtree_impl_t*)fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                                                           tx, 
                                                                                           txtctx, 
                                                                                           btree->m_impl_ref, 
                                                                                           false);
  return fdb_txbtree_get_node(btree, tx, txtctx, btree_impl->m_root, ekey);
}


struct fdb_txpool_alloc_ref_t
fdb_txbtree_split_internal(struct fdb_txbtree_t* btree, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           struct fdb_txpool_alloc_ref_t node_ref, 
                           uint32_t* sibling_key)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         true);

  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  struct fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_create_internal(btree, tx, txtctx);
  uint32_t start = FDB_TXBTREE_INTERNAL_MIN_ARITY;
  *sibling_key = node->m_internal.m_keys[start-1];
  node->m_internal.m_keys[start-1] = 0;
  struct fdb_txbtree_node_t* sibling = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                            tx, 
                                                            txtctx, 
                                                            sibling_ref, 
                                                            true);
  for(uint32_t i = start; i < FDB_TXBTREE_INTERNAL_MAX_ARITY; ++i) 
  {
    sibling->m_internal.m_children[i-start] = node->m_internal.m_children[i];
    sibling->m_internal.m_nchildren++;
    node->m_internal.m_children[i] = (struct fdb_txpool_alloc_ref_t){};
    node->m_internal.m_nchildren--;
  }
  for(uint32_t i = start; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1; ++i ) 
  {
    sibling->m_internal.m_keys[i-start] = node->m_internal.m_keys[i];
    node->m_internal.m_keys[i] = 0;
  }
  return sibling_ref;
}

struct fdb_txpool_alloc_ref_t
fdb_txbtree_split_leaf(struct fdb_txbtree_t* btree, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       struct fdb_txpool_alloc_ref_t node_ref, 
                       uint32_t* sibling_key)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         true);
  FDB_ASSERT(node->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  struct fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_create_leaf(btree, 
                                                               tx, 
                                                               txtctx);
  uint32_t start = FDB_TXBTREE_LEAF_MIN_ARITY;
  *sibling_key = node->m_leaf.m_keys[start];
  struct fdb_txbtree_node_t* sibling = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                            tx, 
                                                            txtctx, 
                                                            sibling_ref, 
                                                            true);
  for(uint32_t i = start; i < FDB_TXBTREE_LEAF_MAX_ARITY; ++i ) 
  {
    sibling->m_leaf.m_leafs[i-start] = node->m_leaf.m_leafs[i];
    sibling->m_leaf.m_keys[i-start] = node->m_leaf.m_keys[i];
    sibling->m_leaf.m_nleafs++;
    node->m_leaf.m_leafs[i] = (struct fdb_txpool_alloc_ref_t){};
    node->m_leaf.m_keys[i] = 0;
    node->m_leaf.m_nleafs--;
  }
  return sibling_ref;
}

void 
fdb_txbtree_shift_insert_internal(struct fdb_txbtree_t* btree, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  struct fdb_txpool_alloc_ref_t node_ref, 
                                  uint32_t idx, 
                                  struct fdb_txpool_alloc_ref_t child_ref, 
                                  uint32_t key)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref,
                                                         true);
  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  FDB_ASSERT(idx > 0 && "Index must be greater than 0");
  for(uint32_t i = FDB_TXBTREE_INTERNAL_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_internal.m_children[i] = node->m_internal.m_children[i-1];
  }

  for(uint32_t i = FDB_TXBTREE_INTERNAL_MAX_ARITY-2; i > idx-1; --i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i-1];
  }
  node->m_internal.m_children[idx] = child_ref;
  node->m_internal.m_keys[idx-1] = key;
  node->m_internal.m_nchildren++;
}

void 
fdb_txbtree_shift_insert_leaf(struct fdb_txbtree_t* btree, 
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              struct fdb_txpool_alloc_ref_t node_ref, 
                              uint32_t idx, 
                              uint32_t key,
                              struct fdb_txpool_alloc_ref_t ptr)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref,
                                                         true);

  FDB_ASSERT(node->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  for(uint32_t i = FDB_TXBTREE_LEAF_MAX_ARITY-1; i > idx; --i) 
  {
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i-1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i-1];
  }

  node->m_leaf.m_leafs[idx] = (struct fdb_txpool_alloc_ref_t){};
  node->m_leaf.m_keys[idx] = key;
  node->m_leaf.m_leafs[idx] = ptr;
  node->m_leaf.m_nleafs++;
}


void 
fdb_txbtree_split_child_full(struct fdb_txbtree_t* btree, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             struct fdb_txpool_alloc_ref_t node_ref, 
                             uint32_t child_idx, 
                             uint32_t key, 
                             struct fdb_txpool_alloc_ref_t* child_ref)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         false);
  *child_ref = node->m_internal.m_children[child_idx];
  struct fdb_txbtree_node_t* child = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                          tx, 
                                                          txtctx, 
                                                          *child_ref, 
                                                          true);
  if(child->m_type == E_TXBTREE_INTERNAL && child->m_internal.m_nchildren == FDB_TXBTREE_INTERNAL_MAX_ARITY) 
  {

    uint32_t sibling_key;
    struct fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_split_internal(btree, 
                                                                           tx, 
                                                                           txtctx,
                                                                           *child_ref, 
                                                                           &sibling_key);

    fdb_txbtree_shift_insert_internal(btree, 
                                      tx, 
                                      txtctx, 
                                      node_ref, 
                                      child_idx+1, 
                                      sibling_ref, 
                                      sibling_key);
    if(key >= sibling_key) 
    {
      *child_ref = sibling_ref;
    }
  } 
  else if( child->m_type == E_TXBTREE_LEAF && child->m_leaf.m_nleafs == FDB_TXBTREE_LEAF_MAX_ARITY )
  {
    uint32_t sibling_key;
    struct fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_split_leaf(btree, 
                                                                       tx, 
                                                                       txtctx,
                                                                       *child_ref, 
                                                                       &sibling_key);

    fdb_txbtree_shift_insert_internal(btree, 
                                      tx, 
                                      txtctx, 
                                      node_ref, 
                                      child_idx+1, 
                                      sibling_ref, 
                                      sibling_key);

    struct fdb_txbtree_node_t* sibling = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                              tx,
                                                              txtctx, 
                                                              sibling_ref, 
                                                              true);
    sibling->m_leaf.m_next = child->m_leaf.m_next;
    child->m_leaf.m_next = sibling_ref;
    if(key >= sibling_key) 
    {
      *child_ref = sibling_ref;
    }
  }
}


struct fdb_txbtree_insert_t 
fdb_txbtree_insert_node(struct fdb_txbtree_t* btree,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        struct fdb_txpool_alloc_ref_t node_ref, 
                        uint32_t key,
                        struct fdb_txpool_alloc_ref_t ptr)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref,
                                                         false);
  if(node->m_type == E_TXBTREE_INTERNAL) 
  {
    uint32_t child_idx = fdb_txbtree_next_internal(btree, tx, txtctx, node_ref, key);
    struct fdb_txpool_alloc_ref_t child_ref = node->m_internal.m_children[child_idx];
    if(child_ref.p_main == NULL) 
    {

      struct fdb_txbtree_node_t* child_zero = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                                   tx,
                                                                   txtctx, 
                                                                   node->m_internal.m_children[0],
                                                                   false);

      if( node->m_internal.m_nchildren == 0 || child_zero->m_type == E_TXBTREE_LEAF)  
      {
        node->m_internal.m_children[child_idx] = fdb_txbtree_create_leaf(btree, 
                                                                         tx, 
                                                                         txtctx);
      } 
      else 
      {
        node->m_internal.m_children[child_idx] = fdb_txbtree_create_internal(btree, 
                                                                             tx, 
                                                                             txtctx);
      }

      node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                  tx, 
                                  txtctx, 
                                  node_ref,
                                  true);
      if(child_idx > 0) 
      {
        node->m_internal.m_keys[child_idx-1] = key;
      }
      node->m_internal.m_nchildren++;
    }

    fdb_txbtree_split_child_full(btree, 
                                 tx, 
                                 txtctx,
                                 node_ref, 
                                 child_idx, 
                                 key, 
                                 &child_ref);
    return fdb_txbtree_insert_node(btree, tx, txtctx, child_ref, key, ptr);
  } 
  else 
  { 
    uint32_t pos = fdb_txbtree_next_leaf(btree, tx, txtctx, node_ref, key);
    if(node->m_leaf.m_keys[pos] != key ||
       node->m_leaf.m_leafs[pos].p_main == NULL)
    {
      fdb_txbtree_shift_insert_leaf(btree, 
                                    tx, 
                                    txtctx,
                                    node_ref, 
                                    pos, 
                                    key, 
                                    ptr);
      struct fdb_txbtree_insert_t insert = {.m_inserted = true, 
                                     .p_place = &node->m_leaf.m_leafs[pos]};
      return insert;
    }
    struct fdb_txbtree_insert_t insert = {.m_inserted = false, 
                                   .p_place = &node->m_leaf.m_leafs[pos]};
    return insert;
  }

  struct fdb_txbtree_insert_t insert = {.m_inserted = false, 
                                 .p_place = NULL};
  return insert;
}

struct fdb_txbtree_insert_t 
fdb_txbtree_insert(struct fdb_txbtree_t* btree, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   uint32_t key, 
                   struct fdb_txpool_alloc_ref_t ptr) 
{
  struct fdb_txbtree_impl_t* btree_impl = fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                               tx,
                                                               txtctx, 
                                                               btree->m_impl_ref, 
                                                               false);
  struct fdb_txpool_alloc_ref_t root_ref = btree_impl->m_root;
  struct fdb_txbtree_node_t* root = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx,
                                                         txtctx, 
                                                         root_ref, 
                                                         false);
  FDB_ASSERT(root->m_type == E_TXBTREE_INTERNAL);
  if(root->m_internal.m_nchildren == FDB_TXBTREE_INTERNAL_MAX_ARITY) 
  {
    root = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                tx,
                                txtctx, 
                                root_ref, 
                                true);
    uint32_t sibling_key;
    struct fdb_txpool_alloc_ref_t sibling_ref = fdb_txbtree_split_internal(btree, 
                                                                           tx, 
                                                                           txtctx,
                                                                           root_ref, 
                                                                           &sibling_key);

    struct fdb_txpool_alloc_ref_t new_root_ref = fdb_txbtree_create_internal(btree, 
                                                                      tx, 
                                                                      txtctx);
    struct fdb_txbtree_node_t* new_root = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                               tx, 
                                                               txtctx,
                                                               new_root_ref, 
                                                               true);
    new_root->m_internal.m_children[0] = root_ref;
    new_root->m_internal.m_children[1] = sibling_ref;
    new_root->m_internal.m_keys[0] = sibling_key;
    new_root->m_internal.m_nchildren = 2;
    btree_impl = fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                          tx,
                                                          txtctx, 
                                                          btree->m_impl_ref, 
                                                          true);
    btree_impl->m_root = new_root_ref;
    root_ref = new_root_ref;
    root = new_root;
  }
  uint32_t child_idx = fdb_txbtree_next_internal(btree, 
                                                 tx, 
                                                 txtctx, 
                                                 root_ref, 
                                                 key);

  struct fdb_txpool_alloc_ref_t child_ref = root->m_internal.m_children[child_idx];
  if(child_ref.p_main == NULL) 
  {
    root = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                tx,
                                txtctx, 
                                root_ref, 
                                true);

    struct fdb_txbtree_node_t* child_zero = NULL;

    if(root->m_internal.m_children[0].p_main)
    {
      child_zero = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator,
                                        tx, 
                                        txtctx, 
                                        root->m_internal.m_children[0], 
                                        false);
    }

    if( root->m_internal.m_nchildren == 0 || 
        child_zero->m_type == E_TXBTREE_LEAF)  
    {
      root->m_internal.m_children[child_idx] = fdb_txbtree_create_leaf(btree, 
                                                                       tx, 
                                                                       txtctx);

    } 
    else 
    {
 
      root->m_internal.m_children[child_idx] =  fdb_txbtree_create_leaf(btree, 
                                                                        tx, 
                                                                        txtctx);
    }

    if(child_idx > 0) 
    {
      root->m_internal.m_keys[child_idx-1] = key;
    }
    root->m_internal.m_nchildren++;
  }

  fdb_txbtree_split_child_full(btree,
                               tx, 
                               txtctx,
                               root_ref, 
                               child_idx, 
                               key, 
                               &child_ref);

  return fdb_txbtree_insert_node(btree,
                                 tx, 
                                 txtctx,
                                 child_ref, 
                                 key, 
                                 ptr);
}

void 
fdb_txbtree_remove_shift_internal(struct fdb_txbtree_t* btree, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  struct fdb_txpool_alloc_ref_t node_ref, 
                                  uint32_t idx) 
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                  tx, 
                                                  txtctx, 
                                                  node_ref, 
                                                  true);
  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  struct fdb_txpool_alloc_ref_t old = node->m_internal.m_children[idx];
  for (uint32_t i = idx; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-1; ++i) 
  { 
    node->m_internal.m_children[i] = node->m_internal.m_children[i+1];
  }
  node->m_internal.m_children[node->m_internal.m_nchildren - 1] = (struct fdb_txpool_alloc_ref_t){};

  uint32_t start = idx==0 ? 0 : idx - 1;
  for (uint32_t i = start; i < FDB_TXBTREE_INTERNAL_MAX_ARITY-2; ++i) 
  {
    node->m_internal.m_keys[i] = node->m_internal.m_keys[i+1];
  }

  node->m_internal.m_nchildren--;
  fdb_txbtree_destroy_node(btree, 
                           tx, 
                           txtctx,
                           old);
}

struct fdb_txpool_alloc_ref_t 
fdb_txbtree_remove_shift_leaf(struct fdb_txbtree_t* btree,
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              struct fdb_txpool_alloc_ref_t node_ref, 
                              uint32_t idx)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         true);
  FDB_ASSERT(node->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  struct fdb_txpool_alloc_ref_t value = node->m_leaf.m_leafs[idx];
  node->m_leaf.m_leafs[idx] = (struct fdb_txpool_alloc_ref_t){};
  for (uint32_t i = idx; i < FDB_TXBTREE_LEAF_MAX_ARITY-1; ++i) 
  { 
    node->m_leaf.m_leafs[i] = node->m_leaf.m_leafs[i+1];
    node->m_leaf.m_keys[i] = node->m_leaf.m_keys[i+1];
  }
  node->m_leaf.m_leafs[node->m_leaf.m_nleafs - 1] = (struct fdb_txpool_alloc_ref_t){};
  node->m_leaf.m_nleafs--;
  return value;
}

void 
fdb_txbtree_merge_internal(struct fdb_txbtree_t* btree, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx, 
                           struct fdb_txpool_alloc_ref_t node_ref, 
                           uint32_t idx1, 
                           uint32_t idx2)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref, 
                                                         false);
  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  struct fdb_txbtree_node_t* child1 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                           tx, 
                                                           txtctx, 
                                                           node->m_internal.m_children[idx1], 
                                                           true);
  struct fdb_txbtree_node_t* child2 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                           tx, 
                                                           txtctx, 
                                                           node->m_internal.m_children[idx2], 
                                                           false);
  uint32_t num_elements1 = child1->m_internal.m_nchildren;
  uint32_t num_elements2 = child2->m_internal.m_nchildren;
  assert(num_elements1 + num_elements2 <= FDB_TXBTREE_INTERNAL_MAX_ARITY);
  for (uint32_t i = 0; i < child2->m_internal.m_nchildren; ++i) 
  {
    child1->m_internal.m_children[child1->m_internal.m_nchildren] = child2->m_internal.m_children[i];
    child1->m_internal.m_nchildren++;
    //fdb_txpool_alloc_ref_nullify(&btree->m_node_allocator, 
    //                             &child2->m_internal.m_children[i]);
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
  fdb_txbtree_remove_shift_internal(btree, tx, txtctx, node_ref, idx2);
}

void 
fdb_txbtree_merge_leaf(struct fdb_txbtree_t* btree, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx, 
                       struct fdb_txpool_alloc_ref_t node_ref, 
                       uint32_t idx1, 
                       uint32_t idx2)
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref,  
                                                         false);
  FDB_ASSERT(node->m_type == E_TXBTREE_INTERNAL && "Node must be an internal node");
  assert(idx2 == idx1+1);
  struct fdb_txbtree_node_t* child1 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                           tx, 
                                                           txtctx, 
                                                           node->m_internal.m_children[idx1], 
                                                           true);
  struct fdb_txbtree_node_t* child2 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator,
                                                           tx, 
                                                           txtctx, 
                                                           node->m_internal.m_children[idx2], 
                                                           false);
  FDB_ASSERT(child1->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  FDB_ASSERT(child2->m_type == E_TXBTREE_LEAF && "Node must be a leaf node");
  for (uint32_t i = 0; i < child2->m_leaf.m_nleafs; ++i) {
    child1->m_leaf.m_leafs[child1->m_leaf.m_nleafs] = child2->m_leaf.m_leafs[i];
    child1->m_leaf.m_keys[child1->m_leaf.m_nleafs] = child2->m_leaf.m_keys[i];
    child1->m_leaf.m_nleafs++;
  }
  child1->m_leaf.m_next = child2->m_leaf.m_next;
  fdb_txbtree_remove_shift_internal(btree, 
                                  tx, 
                                  txtctx,
                                  node_ref, 
                                  idx2);
}

struct fdb_txpool_alloc_ref_t
fdb_txbtree_remove_node(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        struct fdb_txpool_alloc_ref_t node_ref, 
                        uint32_t key, 
                        bool* min_changed, 
                        uint32_t* new_min) 
{
  struct fdb_txbtree_node_t* node = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         node_ref,  
                                                         false);
  *min_changed = false;
  if(node->m_type == E_TXBTREE_INTERNAL) 
  {
    uint32_t child_idx = fdb_txbtree_next_internal(btree, tx, txtctx, node_ref, key);
    struct fdb_txpool_alloc_ref_t child_ref = node->m_internal.m_children[child_idx];
    struct fdb_txpool_alloc_ref_t removed = fdb_txbtree_remove_node(btree, 
                                                                    tx, 
                                                                    txtctx,
                                                                    child_ref, 
                                                                    key, 
                                                                    min_changed, 
                                                                    new_min);
    if(child_idx > 0 && *min_changed) 
    {
      node->m_internal.m_keys[child_idx - 1] = *new_min;
      *min_changed = false;
    }


    struct fdb_txbtree_node_t* child = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                            tx, 
                                                            txtctx, 
                                                            node->m_internal.m_children[child_idx], 
                                                            false);

    if((child->m_type == E_TXBTREE_INTERNAL && child->m_internal.m_nchildren == 0)
       || (child->m_type == E_TXBTREE_LEAF && child->m_leaf.m_nleafs == 0) ) 
    {
      if(child_idx == 0) 
      {
        *new_min = node->m_internal.m_keys[0];
        *min_changed = true;
      }
      fdb_txbtree_remove_shift_internal(btree, 
                                      tx, 
                                      txtctx, 
                                      node_ref, 
                                      child_idx);
    } 

    if(child_idx < FDB_TXBTREE_INTERNAL_MAX_ARITY - 1 && 
       node->m_internal.m_children[child_idx+1].p_main == NULL) 
    {
      struct fdb_txbtree_node_t* child1 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                               tx, 
                                                               txtctx, 
                                                               node->m_internal.m_children[child_idx], 
                                                               false);
      struct fdb_txbtree_node_t* child2 = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                               tx, 
                                                               txtctx, 
                                                               node->m_internal.m_children[child_idx+1],
                                                               false);
      if(child1->m_type == E_TXBTREE_INTERNAL) 
      {
        if(child1->m_internal.m_nchildren + child2->m_internal.m_nchildren <= FDB_TXBTREE_INTERNAL_MAX_ARITY) 
        {
          fdb_txbtree_merge_internal(btree, 
                                   tx, 
                                   txtctx,
                                   node_ref, 
                                   child_idx, 
                                   child_idx+1);
        }
      } 
      else 
      {
        if(child1->m_leaf.m_nleafs + child2->m_leaf.m_nleafs <= FDB_TXBTREE_LEAF_MAX_ARITY) 
        {
          fdb_txbtree_merge_leaf(btree, 
                               tx, 
                               txtctx, 
                               node_ref, 
                               child_idx, 
                               child_idx+1);
        }
      }
    }
    return removed;
  } 
  else 
  { //LEAF
    uint32_t child_idx = fdb_txbtree_next_leaf(btree, tx, txtctx, node_ref, key);
    if(node->m_leaf.m_leafs[child_idx].p_main == NULL || 
       node->m_leaf.m_keys[child_idx] != key) 
    {
      struct fdb_txpool_alloc_ref_t ret = {};
      return ret;
    }
    struct fdb_txpool_alloc_ref_t value = fdb_txbtree_remove_shift_leaf(btree, tx, txtctx, node_ref, child_idx);
    if(child_idx == 0) 
    {
      *min_changed = true;
      *new_min = node->m_leaf.m_keys[child_idx];
    }
    return value;
  }
  struct fdb_txpool_alloc_ref_t ret = {};
  return ret;
}

struct fdb_txpool_alloc_ref_t
fdb_txbtree_remove(struct fdb_txbtree_t* btree, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   uint32_t key) 
{
  bool min_changed;
  uint32_t new_min;
  struct fdb_txbtree_impl_t* btree_impl = (struct fdb_txbtree_impl_t*)fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                                                           tx, 
                                                                                           txtctx, 
                                                                                           btree->m_impl_ref, 
                                                                                           false);

  struct fdb_txpool_alloc_ref_t value = fdb_txbtree_remove_node(btree, 
                                                                tx, 
                                                                txtctx,
                                                                btree_impl->m_root, 
                                                                key, 
                                                                &min_changed, 
                                                                &new_min);

  struct fdb_txbtree_node_t* root = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                         tx, 
                                                         txtctx, 
                                                         btree_impl->m_root, 
                                                         true);

  struct fdb_txbtree_node_t* first_child = fdb_txpool_alloc_ptr(btree->p_factory->p_node_allocator, 
                                                                tx, 
                                                                txtctx, 
                                                                root->m_internal.m_children[0],
                                                                false);
  if(root->m_internal.m_nchildren == 1 && 
     first_child->m_type == E_TXBTREE_INTERNAL) 
  {
    btree_impl = (struct fdb_txbtree_impl_t*)fdb_txpool_alloc_ptr(btree->p_factory->p_btree_allocator, 
                                                                  tx, 
                                                                  txtctx, 
                                                                  btree->m_impl_ref, 
                                                                  true);
    struct fdb_txpool_alloc_ref_t old_root = btree_impl->m_root;
    btree_impl->m_root = root->m_internal.m_children[0];
    root->m_internal.m_nchildren=0;
    root->m_internal.m_children[0] = (struct fdb_txpool_alloc_ref_t){};
    fdb_txbtree_destroy_node(btree, 
                             tx, 
                             txtctx, 
                             old_root);
  }
  return value; 
}
