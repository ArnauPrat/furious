

#ifndef _FURIOUS_TABLE_VIEW_H_
#define _FURIOUS_TABLE_VIEW_H_ value

#include "table.h"

namespace furious {

template<typename TComponent> 
class TableView final  {
  friend class Database;
public:

  struct Row {
    const uint32_t    m_id;
    const TComponent* p_component;
    const bool        m_enabled;
  }; 

  class Iterator final {
  public:
    Iterator(Table::Iterator* it);
    ~Iterator();

    /**
     * @brief Checks whether there is a another component in the table.
     *
     * @return Returns true if there is another component in the table.
     */
    bool has_next() const;

    /**
     * @brief Gets the next row in the table
     *
     * @return Returns the next row in the table. Returns nullptr if it does
     * not exist
     */
    Row next();
    
  private:
    Table::Iterator*  p_table_it;
    TBlockIterator*   p_block_it;
  };

public:
  ~TableView() = default;
private:
  TableView( Table* table );

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
  TComponent* get_element(uint32_t id) const ;

  /**
   * @brief Cre 
   *
   * @tparam TComponent
   * @tparam typename...Args
   * @param id
   * @param ...args
   */
  template<typename...Args>
  void  insert_element(uint32_t id, Args&&...args);

  /**
   * @brief Drops the element with the given id
   *
   * @param id
   */
  void  remove_element(uint32_t id);

  /**
   * @brief Enables an element of the table, only it it exists 
   *
   * @param id The id of the element to enable 
   */
  void enable_element(uint32_t id);

  /**
   * @brief Disables an element of the table
   *
   * @param id The if of the element to disable 
   */
  void disable_element(uint32_t id);

  /**
   * @brief Tells if an element is enabled or not
   *
   * @param id The element to test if it is enabled or not
   *
   * @return True if the element is enabled. False if it is not 
   */
  bool is_enabled(uint32_t id);


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
  Iterator* iterator();

private:
  Table*  p_table;
  
};
  
} /* furious */ 

#include "table_view.inl"

#endif /* ifndef _FURIOUS_TABLE_VIEW_H_ */

