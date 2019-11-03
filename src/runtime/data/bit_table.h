

#ifndef _FURIOUS_BIT_TABLE_H_
#define _FURIOUS_BIT_TABLE_H_

#include "../../common/types.h"
#include "../../common/bitmap.h"
#include "../../common/btree.h"

//#include <bitset>

namespace furious
{

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
   * @brief Gets the bitmap of the bit table for a specific bitmapid.
   *
   * @param bitset_id The bitmap id to retrieve. 
   *
   * @return Returns a pointer to the bitmap
   */
  const Bitmap* 
  get_bitmap(uint32_t bitset_id) const;

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
  mutable BTree<Bitmap> m_bitmaps;
  uint32_t              m_size;
};

void
bittable_union(BitTable* first, const BitTable* second);

void
bittable_difference(BitTable* first, const BitTable* second);
  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_BIT_TABLE_H_ */
