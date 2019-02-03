
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
   * \brief Sets the bit of the given element to the specified value
   *
   * \param element The element to set the bit for
   * \param value The value to set the bit to
   */
  void
  set_bit(uint32_t element, bool value);

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

  /**
   * \brief Gets the maximum number of bits that this bitmap can store
   *
   * \return Returns the maximum number of bits
   */
  uint32_t
  max_bits() const;

  /**
   * \brief Sets the bits based on the given bitmap
   *
   * \param bitmap The bitmap to set the bits from
   */
  void
  set_bitmap(const Bitmap* bitmap);

  /**
   * \brief Ands this bitmap with the given bitmap
   *
   * \param bitmap The bitmap to compute the and with
   */
  void
  set_and(const Bitmap* bitmap);

  /**
   * \brief Ors this bitmap with the given bitmap
   *
   * \param bitmap The bitmap to compute the or with
   */
  void
  set_or(const Bitmap* bitmap);

  /**
   * \brief Negates the contents of this bitmap
   */
  void
  set_negate();

private:
  uint32_t m_max_bits;
  uint32_t m_num_set;
  uint8_t* p_data;
};
  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_BITMAP_H_ */


