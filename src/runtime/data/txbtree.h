
#ifndef _FDB_TXBTREE_H_
#define _FDB_TXBTREE_H_

#include "../../common/memory/memory.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/platform.h"
#include "tx/tx.h"
#include "tx/txpool_allocator.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This is the arity of the txbtree. The arity is choosen in order to make
 * the tree nodes to be close to multiples of cache lines (assuming lines of 64
 * bytes)*/
#define FDB_TXBTREE_INTERNAL_MAX_ARITY 10
#define FDB_TXBTREE_INTERNAL_MIN_ARITY (FDB_TXBTREE_INTERNAL_MAX_ARITY+2-1)/2

#define FDB_TXBTREE_LEAF_MAX_ARITY 9
#define FDB_TXBTREE_LEAF_MIN_ARITY (FDB_TXBTREE_LEAF_MAX_ARITY+2-1)/2

/**
 * \brief This represents a factory of btrees. Multiple btrees created from
 * the same factory will share both the btree and the btree node memory allocators
 */
struct fdb_txbtree_factory_t
{
  struct fdb_txpool_alloc_t*        p_btree_allocator;      // The txpool allocator of the btrees 
  struct fdb_txpool_alloc_t*        p_node_allocator;       // The txpool allocator of the nodes
};

/**
 * \brief A btree
 */
struct fdb_txbtree_t
{
  struct fdb_txbtree_factory_t*  p_factory;    // Pointer to the factory this btree belongs to
  struct fdb_txpool_alloc_ref_t  m_impl_ref;   // Reference to the btree implementation
};

/**
 * \brief This structure is returned as a result of txbtree insert operations
 */
struct fdb_txbtree_insert_t 
{
  bool                            m_inserted; // Whether the element was inserted or not
  struct fdb_txpool_alloc_ref_t*  p_place;    // A pointer to the reference with the inserted element or the element already there
};

/**
 * \brief This structure is returned by txbtree iterators
 */
struct fdb_txbtree_entry_t
{
  uint32_t                        m_key;       // The key of the element
  struct fdb_txpool_alloc_ref_t   m_value_ref; // The reference to the element
};

/**
 * \brief A btree iterator
 */
struct fdb_txbtree_iter_t 
{
  struct fdb_txbtree_t*          p_btree;  // The pointer to the txbtree the iterator belongs to
  struct fdb_tx_t*               p_tx;     // The pointer to the transaction this iterator belongs
  struct fdb_txthread_ctx_t*     p_txtctx; // The pointer to the transaction thread context of the thread using this iteartor
  struct fdb_txpool_alloc_ref_t  m_leaf;   // The reference to the current txbtree leaf
  uint32_t                m_index;  // The index inside the leaf
};

/**
 * \brief The type of btree node
 */
enum fdb_txbtree_node_type_t
{
  E_TXBTREE_INTERNAL = 0,
  E_TXBTREE_LEAF     = 1,
};

struct fdb_txbtree_node_t 
{
  union 
  {
    struct 
    {
      struct fdb_txpool_alloc_ref_t   m_children[FDB_TXBTREE_INTERNAL_MAX_ARITY]; // 10*8 = 80 bytes
      uint32_t                        m_keys[FDB_TXBTREE_INTERNAL_MAX_ARITY-1];   // 9*4 = 36 bytes
      uint16_t                        m_nchildren;                                  // 2 byte
    } m_internal;                                                     // total = 120 bytes

    struct 
    {
      struct fdb_txpool_alloc_ref_t   m_leafs[FDB_TXBTREE_LEAF_MAX_ARITY];  // 9*8 = 72 bytes
      struct fdb_txpool_alloc_ref_t   m_next;                                 // 8 bytes
      uint32_t                        m_keys[FDB_TXBTREE_LEAF_MAX_ARITY];   // 9*4 = 36 bytes
      uint16_t                        m_nleafs;                               // 2 byte
    } m_leaf;                                                 // total 118 bytes

  };
  enum fdb_txbtree_node_type_t    m_type;                      // 1 byte
};

struct fdb_txbtree_impl_t 
{
  struct fdb_txpool_alloc_ref_t    m_root;                 // The root of the txtree
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * \brief Initializes a txbtree factory
 *
 * \param factory The factory to initialize
 * \param mem_allocator The memory allocator to use in the factory. NULL to use
 * the global memory allocator
 */
void
fdb_txbtree_factory_init(struct fdb_txbtree_factory_t* factory, 
                         struct fdb_mem_allocator_t* mem_allocator);

/**
 * \brief Releases a txbtree factory
 *
 * \param factory The factory to release
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 */
void
fdb_txbtree_factory_release(struct fdb_txbtree_factory_t* factory, 
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief inits a txbtree iterator
 *
 * \param iter The iterator to initialize
 * \param btree The txbtree iterator to init
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns a newly initd txbtree iterator
 */
void
fdb_txbtree_iter_init(struct fdb_txbtree_iter_t* iter, 
                      struct fdb_txbtree_t* btree,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Destroys a btree iterator
 *
 * \param iter The btree iterator to destroy
 */
void
fdb_txbtree_iter_release(struct fdb_txbtree_iter_t* iter);

/**
 * \brief Checks if there are more items to iterate
 *
 * \param iter The iterator to check for
 *
 * \return True if there are more items to iterate. False otherwise
 */
bool
fdb_txbtree_iter_has_next(struct fdb_txbtree_iter_t* iter);

/**
 * \brief Returns the next item to iterate
 *
 * \param iter The iterator to get the next item from
 *
 * \return Returns a pointer to the next item
 */
struct fdb_txbtree_entry_t
fdb_txbtree_iter_next(struct fdb_txbtree_iter_t* iter);

/**
 * \brief Initializes a btree 
 *
 * \param btree The txbtree to initialize 
 * \param factory The factory used to initialize the tree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return Returns an instance of a btree root
 */
void
fdb_txbtree_init(struct fdb_txbtree_t* btree, 
                 struct fdb_txbtree_factory_t* factory, 
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Releases a btree 
 *
 * \param root The root instance to destroy
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 */
void
fdb_txbtree_release(struct fdb_txbtree_t* btree, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief inits a new instance of an internal node 
 *
 * \param btree The btree the internal node is initd for
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return The reference to the internal node just created
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_create_internal(struct fdb_txbtree_t* btree, 
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief inits a new instance of a leaf  node 
 *
 * \param btree The btree the leaf node is initd for
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 *
 * \return The reference to the leaf node just created
 *
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_create_leaf(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Destroys a given txbtree node recursively
 *
 * \param btree The btree for which the node must be destroyed
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the node to destroy
 */
void 
fdb_txbtree_destroy_node(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx,
                        struct fdb_txpool_alloc_ref_t node);

/**
 * \brief Given an internal node and a key, finds the index for the child where
 * the given key should exist or be put
 *
 *
 * \param btree The pointer to the btree 
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the internal node
 * \param key The key to look the next for
 *
 * \return Returns the index of the position where the key should lay. 
 */
uint32_t 
fdb_txbtree_next_internal(struct fdb_txbtree_t* btree, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          struct fdb_txpool_alloc_ref_t node, 
                          uint32_t key);

/**
 * \brief Given a leaf node and a key, finds the index for the position where
 * the given key should exist or be put
 *
 * \param btree The pointer to the btree 
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the internal node
 * \param key The key to look the next for
 *
 * \return Returns the index of the position where the key should lay. 
 */
uint32_t 
fdb_txbtree_next_leaf(struct fdb_txbtree_t* btree,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      struct fdb_txpool_alloc_ref_t node, 
                      uint32_t key);

/**
 * \brief Gets the reference to the element with the
 * the given key
 *
 * \param btree The pointer to the btree 
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the internal node
 * \param ekey The key to look the next for
 *
 * \return Returns the reference to the element. The reference is a null
 * reference if the element does not exist
 */
struct fdb_txpool_alloc_ref_t 
fdb_txbtree_get_node(struct fdb_txbtree_t* btree,
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx,
                     struct fdb_txpool_alloc_ref_t node, 
                     uint32_t ekey);

/**
 * \brief Gets the pointer to the value associated with the given key
 *
 * \param btree The btree to get the element from
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param ekey The key of the value to look for
 *
 * \return Returns the reference to the element. The reference is a null
 * reference if the element does not exist
 */
struct fdb_txpool_alloc_ref_t 
fdb_txbtree_get(struct fdb_txbtree_t* btree, 
                struct fdb_tx_t* tx, 
                struct fdb_txthread_ctx_t* txtctx,
                uint32_t ekey);

/**
 * \brief Splits an internal node into two nodes.
 *
 * \param btree The btree the node to split belongs to
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node A reference to the internal node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be init
 *
 * \return The reference to the sibling created
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_split_internal(struct fdb_txbtree_t* btree, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx,
                           struct fdb_txpool_alloc_ref_t node, 
                           uint32_t* sibling_key);

/**
 * \brief Splits a leaf node into two nodes.
 *
 * \param btree The btree the node to split belongs to
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node A reference to the internal node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be init
 *
 * \return The reference to the sibling created
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_split_leaf(struct fdb_txbtree_t* btree, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       struct fdb_txpool_alloc_ref_t node_ref, 
                       uint32_t* sibling_key);

/**
 * \brief Shifts the internal node and inserts the given child into the given
 * position. 
 *
 * \param btree The txbtree the node to split belongs to
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node_ref A reference to the node to insert the child to
 * \param idx The index of the position to start shifting
 * \param child_ref A reference to the child to add at the given position
 * \param key The key of the child to add
 */
void 
fdb_txbtree_shift_insert_internal(struct fdb_txbtree_t* btree, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  struct fdb_txpool_alloc_ref_t node_ref, 
                                  uint32_t idx, 
                                  struct fdb_txpool_alloc_ref_t child_ref, 
                                  uint32_t key);

/**
 * \brief Shifts the leaf node and inserts the given child into the given
 * position
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node_ref A reference to the node to insert the child to
 * \param idx The index of the position to start shifting
 * \param key The key of the child to add
 * \param ptr A reference to the element to add at the given position
 *
 */
void 
fdb_txbtree_shift_insert_leaf(struct fdb_txbtree_t* btree, 
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              struct fdb_txpool_alloc_ref_t node_ref, 
                              uint32_t idx, 
                              uint32_t key,
                              struct fdb_txpool_alloc_ref_t ptr);


/**
 * \brief Inserts an element to the given node. This method assumes that there
 * is free space in the node for inserting the given element
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node_ref A pointer to the node to insert the element to
 * \param key The key of the element to insert
 * \param ptr The reference to the element to insert
 *
 * \return Returns a fdb_btree_insert_t structure.
 */
struct fdb_txbtree_insert_t 
fdb_txbtree_insert_node(struct fdb_txbtree_t* btree,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        struct fdb_txpool_alloc_ref_t node_ref, 
                        uint32_t key,
                        struct fdb_txpool_alloc_ref_t ptr);

/**
 * \brief Inserts a given element to a btree.
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param key The key of the element to insert
 * \param ptr The reference to the element to insert
 *
 * \return Returns a fdb_btree_insert_t structure.
 */
struct fdb_txbtree_insert_t 
fdb_txbtree_insert(struct fdb_txbtree_t* btree, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   uint32_t key,
                   struct fdb_txpool_alloc_ref_t ptr);

/**
 * \brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * \param btree The btree whose node must be rmoved and shifted
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the node to remove the element from
 * \param idx The position of the element to remove
 *
 */
void 
fdb_txbtree_remove_shift_internal(struct fdb_txbtree_t* btree, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  struct fdb_txpool_alloc_ref_t node_ref, 
                                  uint32_t idx);

/**
 * \brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * \param btree The btree whose node must be rmoved and shifted
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node_ref The leaf node to remove the element from
 * \param idx The position of the element to remove
 *
 * \return Returns a reference to the removed element. 
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_remove_shift_leaf(struct fdb_txbtree_t* btree,
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              struct fdb_txpool_alloc_ref_t node_ref, 
                              uint32_t idx);

/**
 * \brief Merges the two internal nodes found at the provided position
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the parent node where the nodes to merge belong to
 * \param idx1 The index of the first node to merge
 * \param idx2 the index of the second node to merge
 */
void 
fdb_txbtree_merge_internal(struct fdb_txbtree_t* btree, 
                           struct fdb_tx_t* tx, 
                           struct fdb_txthread_ctx_t* txtctx, 
                           struct fdb_txpool_alloc_ref_t node_ref, 
                           uint32_t idx1, 
                           uint32_t idx2);

/**
 * \brief Merges the two leaf nodes found at the provided positions
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node The reference to the parent node where the leaf nodes to merge belong to
 * \param idx1 The index of the first node to merge
 * \param idx2 The index of the second node to merge
 */
void 
fdb_txbtree_merge_leaf(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        struct fdb_txpool_alloc_ref_t node, 
                        uint32_t idx1, 
                        uint32_t idx2);

/**
 * \brief Removes an element with the given key
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param node A reference to the node to remove the element from
 * \param key The key of the element to remove
 * \param min_changed Pointer to boolean storing whether the minimum key stored
 * in the subtree changed or not 
 * \param new_min Pointer to store the new minum value of that node 
 *
 * \return Returns a reference to the removed element. A null reference if this
 * element did not exist
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_remove_node(struct fdb_txbtree_t* btree, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        struct fdb_txpool_alloc_ref_t  node_ref, 
                        uint32_t key, 
                        bool* min_changed, 
                        uint32_t* new_min);

/**
 * \brief Removes the element with the given key
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 * \param key The key of the element to remove
 *
 * \return Returns a reference to the removed element. A null reference if this
 * element did not exist
 */
struct fdb_txpool_alloc_ref_t
fdb_txbtree_remove(struct fdb_txbtree_t* btree,
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   uint32_t key);


/**
 * \brief Clears the btree
 *
 * \param btree The txbtree
 * \param tx The transaction 
 * \param txtctx The transaction thread context
 */
void
fdb_txbtree_clear(struct fdb_txbtree_t* btree, 
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_TXBTREE_H_ */
