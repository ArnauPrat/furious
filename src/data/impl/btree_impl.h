

#ifndef _FURIOUS_BTREE_IMPL_H_
#define _FURIOUS_BTREE_IMPL_H_ 

#include "../../common/types.h"

namespace furious {

/**
 * @brief This is the arity of the BTree. The number 7 is purposely choosen,
 * because we assume a cache line of size 64 bytes and we want a node to fit in it.
 * In a line of cache we can fit up to 8
 * pointers of size 8 bytes (8x8=64bytes). Since we need room for the keys,
 * which we assume of size 1 byte, and we want both pointers and keys to fit in
 * a line of cache, we choose 7 pointers and leave the last 8 bytes to store the
 * 6 needed keys, leaving 2 bytes for other stuff. 
 * This btree has a max capacity of 65K objects
 */
constexpr uint32_t BTREE_MAX_ARITY=7;
constexpr uint32_t BTREE_MIN_ARITY=(BTREE_MAX_ARITY+2-1)/2;

enum class BTNodeType : uint8_t {
  E_INTERNAL,
  E_LEAF,
};

struct BTNode {
  union {
    struct __attribute__((__packed__)) {
      BTNode*   m_children[BTREE_MAX_ARITY]; // 7*8 = 56 bytes
      uint8_t   m_keys[BTREE_MAX_ARITY-1];   // 6 bytes
      uint8_t   m_nchildren;                   // 1 byte
    } m_internal; // total 63 bytes

    struct __attribute__((__packed__)) {
      void*       m_leafs[BTREE_MIN_ARITY];  // 4*8 = 32 bytes
      BTNode*     m_next;                    // 8 bytes
      uint8_t     m_keys[BTREE_MIN_ARITY];   // 4 bytes
      uint8_t     m_nleafs;                  // 1 byte
    } m_leaf; // total 45 bytes
  };

  BTNodeType    m_type;                      // 1 byte

};

class BTIterator {
public:
  BTIterator(BTNode* root);
  virtual ~BTIterator() = default;


  /**
   * @brief Tests whether there are more elements or not in the btree
   *
   * @return Returns true if there are more elements in the iterator
   */
  bool has_next();

  /**
   * @brief Gets the next element in the btree
   *
   * @return Returns the next elemnet in the btree
   */
  void* next();

private:
  BTNode* m_root;
  BTNode* m_leaf;
  uint8_t m_index;
};

/**
 * @brief Creates a new instance of an internal node 
 *
 * @return A pointer to the newly created BTNode
 */
BTNode* btree_create_internal();

/**
 * @brief Creates a new instance of a leaf  node 
 *
 * @return A pointer to the newly created BTNode
 */
BTNode* btree_create_leaf();

/**
 * @brief Removes a given BTNode 
 *
 * @param node The pointer to the node to remove
 */
void btree_destroy_node(BTNode* node);

/**
 * @brief Given an internal node and a key, finds the index for the child where
 * the given key should exist or be put
 *
 * @param node The pointer to the internal node
 * @param key The key
 *
 * @return Returns the index of the position where the key should lay. 
 */
uint8_t btree_next_internal(BTNode* node, uint8_t key);

/**
 * @brief Given a leaf node and a key, finds the index for the position where
 * the given key should exist or be put
 *
 * @param node The pointer to the leaf node
 * @param key The key
 *
 * @return Returns the index of the position where the key should lay. 
 */
uint8_t btree_next_leaf(BTNode* node, uint8_t key);

/**
 * @brief Gets the pointer to the value associated with the given key
 *
 * @param node The node to look for the key at
 * @param ekey The key of the value to look for
 *
 * @return Returns the pointer to the value associated with the given key.
 * Returns nullptr if the key does not exist
 */
void* btree_get(BTNode* node, uint8_t ekey);

/**
 * @brief Splits an internal node into two nodes.
 *
 * @param node A pointer to the internal node to split
 * @param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * @return Returns a pointer to the sibling of the split node 
 */
BTNode* btree_split_internal(BTNode* node, uint8_t* sibling_key);

/**
 * @brief Splits a leaf node into two nodes.
 *
 * @param node A pointer to the leaf node to split
 * @param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * @return Returns a pointer to the sibling of the split node 
 */
BTNode* btree_split_leaf(BTNode* node, uint8_t* sibling_key);

/**
 * @brief Shifts the internal node and inserts the given child into the given
 * position
 *
 * @param node A pointer to the node to insert the child to
 * @param idx The index of the position to start shifting
 * @param child A pointer to the child to add at the given position
 * @param key The key of the child to add
 */
void btree_shift_insert_internal(BTNode* node, uint8_t idx, BTNode* child, uint8_t key );

/**
 * @brief Shifts the leaf node and inserts the given child into the given
 * position
 *
 * @param node A pointer to the node to insert the child to
 * @param idx The index of the position to start shifting
 * @param child A pointer to the child to add at the given position
 * @param key The key of the child to add
 */
void btree_shift_insert_leaf(BTNode* node, uint8_t idx, void* element, uint8_t key );


/**
 * @brief Inserts an element to the given node. This method assumes that there
 * is free space in the node for inserting the given element
 *
 * @param node A pointer to the node to insert the element to
 * @param key The key of the element to insert
 * @param element The element to insert
 */
void btree_insert(BTNode* node, uint8_t key, void* element);

/**
 * @brief Inserts a given element to the given node. This method assumes that a
 * pointer to the variable pointing to an nternal node is given. If there is not
 * space to such internal node, the given pointer to pointer will be override
 * with a new value pointing to a new internal node. Effectively, this method is
 * growing the tree from the upper side
 *
 * @param node A pointer to a pointer to an internal node
 * @param key The key of the element to add
 * @param element A pointer to the element to add
 */
void btree_insert_root(BTNode** node, uint8_t key, void* element);

/**
 * @brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * @param node The internal node to remove the element from
 * @param idx The position of the element to remove
 *
 */
void btree_remove_shift_internal(BTNode* node, uint8_t idx);

/**
 * @brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * @param node The leaf node to remove the element from
 * @param idx The position of the element to remove
 *
 */
void btree_remove_shift_leaf(BTNode* node, uint8_t idx);

/**
 * @brief Merges the two internal nodes found at the provided position
 *
 * @param node The parent node where the nodes to merge belong to
 * @param idx1 The index of the first node to merge
 * @param idx2 the index of the second node to merge
 */
void btree_merge_internal(BTNode* node, uint8_t idx1, uint8_t idx2);

/**
 * @brief Merges the two leaf nodes found at the provided positions
 *
 * @param node The parent node where the leaf nodes to merge belong to
 * @param idx1 The index of the first node to merge
 * @param idx2 The index of the second node to merge
 */
void btree_merge_internal(BTNode* node, uint8_t idx1, uint8_t idx2);

/**
 * @brief Removes an element with the given key
 *
 * @param node A pointer to the node to remove the element from
 * @param key The key of the element to remove
 * @param changed_min Pointer to boolean storing whether the minimum changed or
 * not 
 * @param new_key Pointer to store the new minum value of that node 
 *
 * @return Returns a pointer to the removed element
 */
void* btree_remove(BTNode* node, uint8_t key, bool* min_changed, uint8_t* new_min);

/**
 * @brief Removes the element with the given key
 *
 * @param node A pointer to a pointer to the internal root node of the btree. If
 * the remove operation makes it possible to reduce the tree height, the
 * internal node pointer by node is replaced.
 * @param key The key of the element to remove
 *
 * @return Returns a pointer to the removed element. Returns nullptr if the
 * element was not found.
 */
void* btree_remove(BTNode** node, uint8_t key);
  
} /* btree_impl */ 

#endif /* ifndef _FURIOUS_BTREE_IMPL_H_ */
