

#ifndef _FURIOUS_BTREE_H_
#define _FURIOUS_BTREE_H_ value
#include "../common/types.h"
#include <stdlib.h>

namespace furious {

struct BTNode;
class BTIterator;


template<typename T>
class BTree {

public:
  class Iterator {
  public:
    Iterator(BTNode* root);
    virtual ~Iterator();
    bool has_next();
    T* next();

    Iterator(const Iterator& it) = delete;
    void operator=(const Iterator& it) = delete;
    
  private:
    BTIterator* m_iterator;
  };

public: 
  BTree();
  BTree(const BTree&) = delete;
  BTree(BTree&&) = delete;
  virtual ~BTree();
  
  void operator=(const BTree&) = delete;
  void operator=(BTree&&) = delete;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  

  /**
   * @brief Inserts an element to the btree. Replaces the element if this
   * already existed
   *
   * @param key The key of the element to add 
   * @param element A pointer to the element to insert
   *
   * @return Returns a pointer to the replaced element. nullptr otherwise.
   */
  void insert(uint8_t key, T* element);
  

  /**
   * @brief Removes an element from the tree
   *
   * @param key The key of the element to remove 
   *
   * @return Returns a pointer to the removed element
   */
  T* remove(uint8_t key);

  /**
   * @brief Gets the element with the given key
   *
   * @param key The key of the element to search for
   *
   * @return Returns a pointer to the element with the given key. nullptr if it
   * does not exist
   */
  T* get(uint8_t key);

  /**
   * @brief Checks if an element exists
   *
   * @param key The key to of the element to look for
   *
   * @return Retursn true if the element exists. false otherwise
   */
  bool exists(uint8_t key);

  /**
   * @brief Gets the size of the btree (i.e. the number of elements)
   *
   * @return Returns the size of the btree
   */
  size_t size();

  /**
   * @brief Gets an iterator to the btree
   *
   * @return Returns an iterator to the btree
   */
  Iterator* iterator();

  /**
   * @brief Clears the btree
   */
  void clear();

private:
   BTNode*    p_root;
   size_t     m_size;
};

  
} /* furious */ 

#include "btree.inl"
#endif /* ifndef  _FURIOUS_BTREE_H_ */
