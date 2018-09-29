

#ifndef _FURIOUS_BIT_TABLE_H_
#define _FURIOUS_BIT_TABLE_H_

#include "../../common/common.h"
#include "table.h"

//#include <bitset>

namespace furious
{

class BitTable {
public:
  BitTable () = default;
  virtual ~BitTable () = default;

  /**
   * @brief Tests if an element exists in the bit table
   *
   * @param id The id of the element to test
   *
   * @return Returns true if the element exists. false otherwise.
   */
  bool
  exists(int32_t id) const;

  /**
   * @brief Adds an element to the bit table 
   *
   * @param id
   */
  void
  add(int32_t id);

  /**
   * @brief Removes an element from the bit table
   *
   * @param id The id of the element to remove
   */
  void
  remove(int32_t id);

  /**
   * @brief Gets the bitset of the bit table for a specific bitsetid.
   *
   * @param bitset_id The bitset id to retrieve. 
   *
   * @return Returns a reference to the bitset
   */
  const std::bitset<TABLE_BLOCK_SIZE>&
  get_bitset(int32_t bitset_id) const;

private:
  mutable std::unordered_map<int32_t, std::bitset<TABLE_BLOCK_SIZE>> m_bitsets;
};
  
} /* furious
 */ 
#endif /* ifndef _FURIOUS_BIT_TABLE_H_ */
