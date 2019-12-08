

#ifndef _FURIOUS_BTREE_H_
#define _FURIOUS_BTREE_H_ value

#include "types.h"
#include "impl/btree_impl.h"

#include <stdlib.h>

#include <mutex>

namespace furious 
{

template<typename T>
struct BTree 
{
  struct Entry
  {
    uint32_t    m_key;
    T*          p_value;
  };

  struct Iterator 
  {
  public:
    Iterator(btree_t* root);
    ~Iterator() = default;

    bool 
    has_next() const;

    Entry 
    next();
    
  private:
    BTIterator m_iterator;
  };

  BTree();
  BTree(const BTree&) = delete;
  BTree(BTree&&) = delete;
  ~BTree();
  
  void operator=(const BTree&) = delete;
  void operator=(BTree&&) = delete;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  

  /**
   * \brief Inserts an element to the btree. Replaces the element if this
   * already existed
   *
   * \param key The key of the element to add 
   * \param element A pointer to the element to insert
   *
   * \return Returns a pointer to the inserted element or nullptr otherwise.
   */
  T* 
  insert_copy(uint32_t key, const T* element);


  template <typename...Args>
  T* 
  insert_new(uint32_t key, Args &&... args);
  

  /**
   * \brief Removes an element from the tree
   *
   * \param key The key of the element to remove 
   *
   */
  void 
  remove(uint32_t key);

  /**
   * \brief Gets the element with the given key
   *
   * \param key The key of the element to search for
   *
   * \return Returns a pointer to the element with the given key. nullptr if it
   * does not exist
   */
  T* 
  get(uint32_t key) const;

  /**
   * \brief Checks if an element exists
   *
   * \param key The key to of the element to look for
   *
   * \return Retursn true if the element exists. false otherwise
   */
  bool 
  exists(uint32_t key);

  /**
   * \brief Gets the size of the btree (i.e. the number of elements)
   *
   * \return Returns the size of the btree
   */
  size_t
  size() const;

  /**
   * \brief Gets an iterator to the btree
   *
   * \return Returns an iterator to the btree
   */
  Iterator 
  iterator() const;

  /**
   * brief Clears the btree
   */
  void clear();

private:
   btree_t*     p_root;
   size_t       m_size;
};

} /* furious */ 

#include "btree.inl"
#endif /* ifndef  _FURIOUS_BTREE_H_ */
