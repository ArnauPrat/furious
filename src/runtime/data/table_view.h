

#ifndef _FURIOUS_TABLE_VIEW_H_
#define _FURIOUS_TABLE_VIEW_H_ value

#include "table.h"

namespace furious {

template<typename TComponent> 
class TableView final  {
public:

    class Block {
    public:
      typedef TComponent type;
      Block(TBlock* block);
      ~Block() = default;

      TComponent* get_data() const;

      size_t      get_num_elements() const;

      size_t      get_size() const;

      int32_t     get_start() const;

      const std::bitset<TABLE_BLOCK_SIZE>&  get_enabled() const;

      TBlock*     get_raw() const;

    private:
      TBlock* p_tblock;
    };

    class BlockIterator {
    public:

      BlockIterator(Table::Iterator iter);
      ~BlockIterator() = default;

      Block next();

      bool has_next() const;

    private:
      Table::Iterator m_iterator;

    };

  TableView( Table* table );
  ~TableView() = default;

public:
  /**
   * @brief Clears the table
   */
  void clear();

  /**
   * @brief Gets the element with the given id
   *
   * @param id The id of the element to get
   *
   * @return Returns a pointer to the element. Returns nullptr if the element
   * does not exist in the table
   */
  TComponent* get_element(int32_t id) const ;

  /**
   * @brief Cre 
   *
   * @tparam TComponent
   * @tparam typename...Args
   * @param id
   * @param ...args
   */
  template<typename...Args>
  void  insert_element(int32_t id, Args&&...args);

  /**
   * @brief Drops the element with the given id
   *
   * @param id
   */
  void remove_element(int32_t id);

  /**
   * @brief Enables an element of the table, only it it exists 
   *
   * @param id The id of the element to enable 
   */
  void enable_element(int32_t id);

  /**
   * @brief Disables an element of the table
   *
   * @param id The if of the element to disable 
   */
  void disable_element(int32_t id);

  /**
   * @brief Tells if an element is enabled or not
   *
   * @param id The element to test if it is enabled or not
   *
   * @return True if the element is enabled. False if it is not 
   */
  bool is_enabled(int32_t id);


  /**
   * @brief Gets the size of the table
   *
   * @return Returns the size of the table
   */
  size_t size() const;

  /**
   * @brief Gets an iterator of the table
   *
   * @return A new iterator of the table. 
   */
  BlockIterator iterator();

private:
  Table*  p_table;
};
  
} /* furious */ 

#include "table_view.inl"

#endif /* ifndef _FURIOUS_TABLE_VIEW_H_ */

