
#ifndef _FURIOUS_BITMAP_H_
#define _FURIOUS_BITMAP_H_ value

#include "types.h"

namespace furious
{

struct Bitmap
{
  Bitmap(uint32_t num_elements);
  ~Bitmap();

  /**
   * \brief Sets to one the ith element of the bitmap
   *
   * \param element The element to set
   */
  void
  set(uint32_t element);

  /**
   * \brief Sets to 0 the ith element of the bitmap
   *
   * \param element The ith element to set
   */
  void
  unset(uint32_t element);

  /**
   * \brief Checks if a given element is set to 1
   *
   * \param element The element to check
   *
   * \return True if the element is set to 1
   */
  bool
  is_set(uint32_t element) const;

  /**
   * \brief The number of elements set to 1
   *
   * \return Returns the number of elements set to one
   */
  uint32_t
  num_set() const;

private:
  uint32_t m_max_bits;
  uint32_t m_num_set;
  uint8_t* p_data;
};
  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BITMAP_H_ */


