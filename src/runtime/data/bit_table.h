

#ifndef _FURIOUS_BIT_TABLE_H_
#define _FURIOUS_BIT_TABLE_H_

#include "../../common/types.h"
#include "../../common/bitmap.h"
#include <unordered_map>

//#include <bitset>

namespace furious
{

class BitTable {
public:
  BitTable () = default;
  ~BitTable ();

  /**
   * @brief Tests if an element exists in the bit table
   *
   * @param id The id of the element to test
   *
   * @return Returns true if the element exists. false otherwise.
   */
  bool
  exists(uint32_t id) const;

  /**
   * @brief Adds an element to the bit table 
   *
   * @param id
   */
  void
  add(uint32_t id);

  /**
   * @brief Removes an element from the bit table
   *
   * @param id The id of the element to remove
   */
  void
  remove(uint32_t id);

  /**
   * @brief Gets the bitmap of the bit table for a specific bitmapid.
   *
   * @param bitset_id The bitmap id to retrieve. 
   *
   * @return Returns a pointer to the bitmap
   */
  const Bitmap* 
  get_bitmap(uint32_t bitset_id) const;

private:
  mutable std::unordered_map<uint32_t, Bitmap*> m_bitsets;
};
  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_BIT_TABLE_H_ */
