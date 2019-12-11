

#ifndef _FURIOUS_BIT_TABLE_H_
#define _FURIOUS_BIT_TABLE_H_

#include "../../common/types.h"
#include "../../common/bitmap.h"
#include "../../common/btree.h"

#include "common.h"


namespace furious
{

using bt_block_t = bitmap_t<TABLE_BLOCK_SIZE>;

struct BitTable 
{
  friend void bittable_union(BitTable* first, const BitTable* second);
  friend void bittable_difference(BitTable* first, const BitTable* second);

  BitTable ();
  ~BitTable ();

  /**
   * @brief Tests if an element exists in the bit table
   *
   * @param id The id of the element to test
   *
   * @return Returns true if the element exists. false otherwise.
   */
  bool
  exists(entity_id_t id) const;

  /**
   * @brief Adds an element to the bit table 
   *
   * @param id
   */
  void
  add(entity_id_t id);

  /**
   * @brief Removes an element from the bit table
   *
   * @param id The id of the element to remove
   */
  void
  remove(entity_id_t id);

  /**
   * @brief Gets the bitmap of the bit table for a specific entity id.
   *
   * @param id The id of the entity to get the bitmap of 
   *
   * @return Returns a pointer to the bitmap. Returns nullptr if the bitmap does
   * not exist.
   */
  const bt_block_t* 
  get_bitmap(entity_id_t id) const;

  /**
   * \brief Gets the size of the bittable (number of bits set to 1)
   *
   * \return  The size of the bit table
   */
  uint32_t
  size();

  /**
   * \brief Clears the bittable
   */
  void
  clear();

private:

  enum class logic_operation_t
  {
    E_AND,
    E_OR,
    E_DIFF
  };

  /**
   * \brief Adds a bitmap  into the bittable
   *
   * \param id The identifier of the bitmap
   * \param bitmap The bitmap to add
   * \param operation The operation to perform
   */
  void
  apply_bitset(uint32_t id, 
               const bt_block_t* bitmap,
               logic_operation_t operation);

  /**
   * \brief Gets the underlying bitmap identified by the given id
   *
   * \param bitset_id The bitset_id to get
   *
   * \return 
   */
  bt_block_t*
  get_bitset(uint32_t bitset_id) const;


  btree_t*       m_bitmaps;
  uint32_t      m_size;
};

void
bittable_union(BitTable* first, const BitTable* second);

void
bittable_difference(BitTable* first, const BitTable* second);
  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_BIT_TABLE_H_ */
