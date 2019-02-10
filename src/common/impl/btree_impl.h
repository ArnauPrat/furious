

#ifndef _FURIOUS_BTREE_IMPL_H_
#define _FURIOUS_BTREE_IMPL_H_ 

#include "../types.h"
#include "stdlib.h"

namespace furious 
{

/**
 * \brief This is the arity of the BTree. The arity is choosen in order to make
 * the tree nodes to be close to multiples of cache lines (assuming lines of 64
 * bytes)*/
constexpr uint32_t BTREE_INTERNAL_MAX_ARITY=10;
constexpr uint32_t BTREE_INTERNAL_MIN_ARITY=(BTREE_INTERNAL_MAX_ARITY+2-1)/2;

constexpr uint32_t BTREE_LEAF_MAX_ARITY=9;
constexpr uint32_t BTREE_LEAF_MIN_ARITY=(BTREE_LEAF_MAX_ARITY+2-1)/2;

enum class BTNodeType : uint8_t 
{
  E_INTERNAL,
  E_LEAF,
};

struct BTNode 
{
  union 
  {
    struct 
    {
      BTNode*     m_children[BTREE_INTERNAL_MAX_ARITY]; // 10*8 = 80 bytes
      uint32_t    m_keys[BTREE_INTERNAL_MAX_ARITY-1];   // 9*4 = 36 bytes
      uint16_t    m_nchildren;                          // 2 byte
    } m_internal;                                       // total = 120 bytes

    struct 
    {
      void*     m_leafs[BTREE_LEAF_MAX_ARITY];  // 9*8 = 72 bytes
      BTNode*   m_next;                         // 8 bytes
      uint32_t  m_keys[BTREE_LEAF_MAX_ARITY];   // 9*4 = 36 bytes
      uint16_t  m_nleafs;                       // 2 byte
    } m_leaf;                                   // total 118 bytes
  };

  BTNodeType    m_type;                      // 1 byte
};

struct BTRoot
{
  BTNode* p_root;                 // The root of the tree
};

struct BTInsert
{
  bool    m_inserted;
  void**  p_place;
};

/**
 * \brief Allocates memory for one element
 *
 * \param root The BTRoot to allocate memory for
 *
 * \return Returns the pointer to the newly allocated memory
 */
void*
alloc(BTRoot* root);

struct BTIterator 
{
  BTIterator(BTRoot* root);
  ~BTIterator() = default;


  /**
   * \brief Tests whether there are more elements or not in the btree
   *
   * \return Returns true if there are more elements in the iterator
   */
  bool 
  has_next() const;

  /**
   * \brief Gets the next element in the btree
   *
   * \return Returns the next elemnet in the btree
   */
  void* 
  next();

private:
  BTRoot*   m_root;
  BTNode*   m_leaf;
  uint32_t  m_index;
};

/**
 * \brief Creates a btree root
 *
 * \param element_size The size of the elements to store
 *
 * \return Returns an instance of a btree root
 */
BTRoot*
btree_create_root();

/**
 * \brief Destroys an btree root instance
 *
 * \param root The root instance to destroy
 */
void
btree_destroy_root(BTRoot* root);

/**
 * \brief Creates a new instance of an internal node 
 *
 * \return A pointer to the newly created BTNode
 */
BTNode* 
btree_create_internal();

/**
 * \brief Creates a new instance of a leaf  node 
 *
 * \return A pointer to the newly created BTNode
 */
BTNode*
btree_create_leaf();

/**
 * \brief Removes a given BTNode 
 *
 * \param node The pointer to the node to remove
 */
void 
btree_destroy_node(BTNode* node);

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
btree_next_internal(BTNode* node, 
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
btree_next_leaf(BTNode* node, 
                uint32_t key);

/**
 * \brief Gets the index to the data buffer where the element with the
 * the given key resides
 *
 * \param node The node to look for the key at
 * \param ekey The key of the value to look for
 *
 * \return Returns a pointer the memory region where element resides.
 * nullptr if the element does not exist
 */
void* 
btree_get(BTNode* node, 
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
btree_get_root(BTRoot* root, 
               uint32_t ekey);

/**
 * \brief Splits an internal node into two nodes.
 *
 * \param node A pointer to the internal node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * \return Returns a pointer to the sibling of the split node 
 */
BTNode* 
btree_split_internal(BTNode* node, 
                     uint32_t* sibling_key);

/**
 * \brief Splits a leaf node into two nodes.
 *
 * \param node A pointer to the leaf node to split
 * \param sibling_key A pointer to the variable to store the key of the sibling
 * to be created
 *
 * \return Returns a pointer to the sibling of the split node 
 */
BTNode* 
btree_split_leaf(BTNode* node, 
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
btree_shift_insert_internal(BTNode* node, 
                            uint32_t idx, 
                            BTNode* child, 
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
btree_shift_insert_leaf(BTRoot* root,
                        BTNode* node, 
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
 * \return Returns a BTInsert structure.
 */
BTInsert 
btree_insert(BTRoot* root,
             BTNode* node, 
             uint32_t key);

/**
 * \brief Inserts a given element to the given node. This method assumes that a
 * pointer to the variable pointing to an nternal node is given. If there is not
 * space to such internal node, the given pointer to pointer will be override
 * with a new value pointing to a new internal node. Effectively, this method is
 * growing the tree from the upper side
 *
 * \param node A pointer to a pointer to an internal node
 * \param key The key of the element to add
 *
 * \return Returns a BTInsert structure.
 */
BTInsert 
btree_insert_root(BTRoot* node, 
                  uint32_t key);

/**
 * \brief Removes the element at position idx and shifts the rest of elements to
 * left
 *
 * \param node The internal node to remove the element from
 * \param idx The position of the element to remove
 *
 */
void 
btree_remove_shift_internal(BTNode* node, 
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
btree_remove_shift_leaf(BTNode* node, 
                        uint32_t idx);

/**
 * \brief Merges the two internal nodes found at the provided position
 *
 * \param node The parent node where the nodes to merge belong to
 * \param idx1 The index of the first node to merge
 * \param idx2 the index of the second node to merge
 */
void 
btree_merge_internal(BTNode* node, 
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
btree_merge_internal(BTNode* node, 
                     uint32_t idx1, 
                     uint32_t idx2);

/**
 * \brief Removes an element with the given key
 *
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
btree_remove(BTNode* node, 
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
btree_remove_root(BTRoot* root,
                  uint32_t key);
  
} /* btree_impl */ 

#endif /* ifndef _FURIOUS_BTREE_IMPL_H_ */
