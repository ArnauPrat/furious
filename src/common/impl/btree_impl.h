

#ifndef _FURIOUS_BTREE_IMPL_H_
#define _FURIOUS_BTREE_IMPL_H_ 

#include "../memory/memory.h"
#include "../types.h"
#include "stdlib.h"

namespace furious 
{

/**
 * \brief This is the arity of the BTree. The arity is choosen in order to make
 * the tree nodes to be close to multiples of cache lines (assuming lines of 64
 * bytes)*/
constexpr uint32_t FURIOUS_BTREE_INTERNAL_MAX_ARITY=10;
constexpr uint32_t FURIOUS_BTREE_INTERNAL_MIN_ARITY=(FURIOUS_BTREE_INTERNAL_MAX_ARITY+2-1)/2;

constexpr uint32_t FURIOUS_BTREE_LEAF_MAX_ARITY=9;
constexpr uint32_t FURIOUS_BTREE_LEAF_MIN_ARITY=(FURIOUS_BTREE_LEAF_MAX_ARITY+2-1)/2;

enum class btree_node_type_t : uint8_t 
{
  E_INTERNAL = 0,
  E_LEAF     = 1,
};

struct btree_node_t 
{
  union 
  {
    struct 
    {
      btree_node_t*     m_children[FURIOUS_BTREE_INTERNAL_MAX_ARITY]; // 10*8 = 80 bytes
      uint32_t          m_keys[FURIOUS_BTREE_INTERNAL_MAX_ARITY-1];   // 9*4 = 36 bytes
      uint16_t          m_nchildren;                                  // 2 byte
    } m_internal;                                                     // total = 120 bytes

    struct 
    {
      void*           m_leafs[FURIOUS_BTREE_LEAF_MAX_ARITY];  // 9*8 = 72 bytes
      btree_node_t*   m_next;                                 // 8 bytes
      uint32_t        m_keys[FURIOUS_BTREE_LEAF_MAX_ARITY];   // 9*4 = 36 bytes
      uint16_t        m_nleafs;                               // 2 byte
    } m_leaf;                                                 // total 118 bytes

  };

  btree_node_type_t    m_type;                      // 1 byte
};

struct btree_t 
{
  btree_node_t*   p_root;                 // The root of the tree
  mem_allocator_t m_allocator;
};

struct btree_insert_t 
{
  bool    m_inserted;
  void**  p_place;
};

struct btree_entry_t
{
  uint32_t    m_key;
  void*       p_value;
};

struct btree_iter_t 
{
  btree_t*        p_root;
  btree_node_t*   p_leaf;
  uint32_t        m_index;
};

/**
 * \brief Creates a btree iterator
 *
 * \param btree The btree iterator to create
 *
 * \return Returns a newly created btree iterator
 */
btree_iter_t
btree_iter_create(btree_t* btree);

/**
 * \brief Destroys a btree iterator
 *
 * \param iter The btree iterator to destroy
 */
void
btree_iter_destroy(btree_iter_t* iter);

/**
 * \brief Checks if there are more items to iterate
 *
 * \param iter The iterator to check for
 *
 * \return True if there are more items to iterate. False otherwise
 */
bool
btree_iter_has_next(btree_iter_t* iter);

/**
 * \brief Returns the next item to iterate
 *
 * \param iter The iterator to get the next item from
 *
 * \return Returns a pointer to the next item
 */
btree_entry_t
btree_iter_next(btree_iter_t* iter);

/**
 * \brief Initializes a btree 
 *
 * \param allocator The mem allocator to use
 *
 * \return Returns an instance of a btree root
 */
btree_t 
btree_create(mem_allocator_t* allocator = nullptr);

/**
 * \brief Releases a btree 
 *
 * \param root The root instance to destroy
 */
void
btree_destroy(btree_t* root);

/**
 * \brief Creates a new instance of an internal node 
 *
 * \param btree The btree the internal node is created for
 *
 * \return A pointer to the newly created btree_node_t
 */
btree_node_t* 
btree_create_internal(btree_t* btree);

/**
 * \brief Creates a new instance of a leaf  node 
 *
 * \param btree The btree the leaf node is created for
 *
 * \return A pointer to the newly created btree_node_t
 */
btree_node_t*
btree_create_leaf(btree_t* btree);

/**
 * \brief Removes a given btree_node_t 
 *
 * \param btree The btree for which the node must be destroyed
 *
 * \param node The pointer to the node to remove
 */
void 
btree_destroy_node(btree_t* btree, 
                   btree_node_t* node);

/**
 * \brief Given an internal node and a key, finds the index for the child where
 * the given key should exist or be put
 *
 * \param node The pointer to the internal node
 * \param key The key
 *
 * \return Returns the index of the position where the key should lay. 
 */
uint32_t 
btree_next_internal(btree_node_t* node, 
                    uint32_t key);

/**
 * \brief Given a leaf node and a key, finds the index for the position where
 * the given key should exist or be put
 *
 * \param node The pointer to the leaf node
 * \param key The key
 *
 * \return Returns the index of the position where the key should lay. 
 */
uint32_t 
btree_next_leaf(btree_node_t* node, 
                uint32_t key);

/**
 * \brief Gets the data pointer where the element with the
 * the given key resides
 *
 * \param node The node to look for the key at
 * \param ekey The key of the value to look for
 *
 * \return Returns a pointer the memory region where element resides.
 * nullptr if the element does not exist
 */
void* 
btree_get_node(btree_node_t* node, 
          uint32_t ekey);

/**
 * \brief Gets the pointer to the value associated with the given key
 *
 * \param node The node to look for the key at
 * \param ekey The key of the value to look for
 *
 * \return Returns a pointer to the element with the given key.
 * Returns nullptr if the element does not exist
 */
void* 
btree_get(btree_t* root, 
          uint32_t ekey);

/**
 * \brief Splits an internal node into two nodes.
 *
 * \param btree The btree the node to split belongs to
 * \param node A pointer to the internal node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * \return Returns a pointer to the sibling of the split node 
 */
btree_node_t* 
btree_split_internal(btree_t* btree, 
                     btree_node_t* node, 
                     uint32_t* sibling_key);

/**
 * \brief Splits a leaf node into two nodes.
 *
 * \param btree The btree the node to split belongs to
 * \param node A pointer to the leaf node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * \return Returns a pointer to the sibling of the split node 
 */
btree_node_t* 
btree_split_leaf(btree_t* btree, 
                 btree_node_t* node, 
                 uint32_t* sibling_key);

/**
 * \brief Shifts the internal node and inserts the given child into the given
 * position
 *
 * \param node A pointer to the node to insert the child to
 * \param idx The index of the position to start shifting
 * \param child A pointer to the child to add at the given position
 * \param key The key of the child to add
 */
void 
btree_shift_insert_internal(btree_node_t* node, 
                            uint32_t idx, 
                            btree_node_t* child, 
                            uint32_t key);

/**
 * \brief Shifts the leaf node and inserts the given child into the given
 * position
 *
 * \param node A pointer to the node to insert the child to
 * \param idx The index of the position to start shifting
 * \param child A pointer to the child to add at the given position
 * \param key The key of the child to add
 *
 * \return Returns a pointer to the reserved memory for the inserted element 
 */
void 
btree_shift_insert_leaf(btree_t* root,
                        btree_node_t* node, 
                        uint32_t idx, 
                        uint32_t key);


/**
 * \brief Inserts an element to the given node. This method assumes that there
 * is free space in the node for inserting the given element
 *
 * \param node A pointer to the node to insert the element to
 * \param key The key of the element to insert
 * \param element The element to insert
 * \param place A pointer to the location where the pointer to the element to
 * insert should be written 
 *
 * \return Returns a btree_insert_t structure.
 */
btree_insert_t 
btree_insert_node(btree_t* root,
                  btree_node_t* node, 
                  uint32_t key);

/**
 * \brief Inserts a given element to the given node. This method assumes that a
 * pointer to the variable pointing to an nternal node is given. If there is not
 * space to such internal node, the given pointer to pointer will be overriden
 * with a new value pointing to a new internal node. Effectively, this method is
 * growing the tree from the upper side
 *
 * \param node A pointer to a pointer to an internal node
 * \param key The key of the element to add
 *
 * \return Returns a btree_insert_t structure.
 */
btree_insert_t 
btree_insert(btree_t* node, 
             uint32_t key);

/**
 * \brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * \param btree The btree whose node must be rmoved and shifted
 * \param node The internal node to remove the element from
 * \param idx The position of the element to remove
 *
 */
void 
btree_remove_shift_internal(btree_t* btree, 
                            btree_node_t* node, 
                            uint32_t idx);

/**
 * \brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * \param node The leaf node to remove the element from
 * \param idx The position of the element to remove
 *
 * \return Returns a pointer to the removed element. 
 */
void* 
btree_remove_shift_leaf(btree_node_t* node, 
                        uint32_t idx);

/**
 * \brief Merges the two internal nodes found at the provided position
 *
 * \param node The parent node where the nodes to merge belong to
 * \param idx1 The index of the first node to merge
 * \param idx2 the index of the second node to merge
 */
void 
btree_merge_internal(btree_t* btree, 
                     btree_node_t* node, 
                     uint32_t idx1, 
                     uint32_t idx2);

/**
 * \brief Merges the two leaf nodes found at the provided positions
 *
 * \param node The parent node where the leaf nodes to merge belong to
 * \param idx1 The index of the first node to merge
 * \param idx2 The index of the second node to merge
 */
void 
btree_merge_leaf(btree_node_t* node, 
                 uint32_t idx1, 
                 uint32_t idx2);

/**
 * \brief Removes an element with the given key
 *
 * \param btree The btree whose node must be removed
 * \param node A pointer to the node to remove the element from
 * \param key The key of the element to remove
 * \param changed_min Pointer to boolean storing whether the minimum key stored
 * in the subtree changed or not 
 * \param new_key Pointer to store the new minum value of that node 
 *
 * \return Returns a pointer to the memory region where the element is stored.
 * nullptr if this does not exist
 */
void*
btree_remove_node(btree_t* btree, 
                  btree_node_t* node, 
                  uint32_t key, 
                  bool* min_changed, 
                  uint32_t* new_min);

/**
 * \brief Removes the element with the given key
 *
 * \param node A pointer to a pointer to the internal root node of the btree. If
 * the remove operation makes it possible to reduce the tree height, the
 * internal node pointer by node is replaced.
 * \param key The key of the element to remove
 *
 * \return Returns a pointer to the memory region where the element is stored.
 * nullptr if this does not exist
 */
void* 
btree_remove(btree_t* root,
             uint32_t key);
  
} /* btree_impl */ 

#endif /* ifndef _FURIOUS_BTREE_IMPL_H_ */
