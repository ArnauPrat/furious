

#ifndef _FURIOUS_TABLE_VIEW_H_
#define _FURIOUS_TABLE_VIEW_H_ value

#include "table.h"

namespace furious 
{

template<typename TComponent> 
struct TableView   
{
    struct Block 
    {
      typedef TComponent type;
      Block(table_block_t* block);
      ~Block() = default;

      TComponent* 
      get_data() const;

      size_t      
      get_num_components() const;

      size_t      
      get_size() const;

      entity_id_t     
      get_start() const;

      const bitmap_t* 
      get_enabled() const;

      table_block_t*     
      get_raw() const;

    private:
      table_block_t* p_table_block_t;
    };

    struct BlockIterator 
    {
      BlockIterator(table_iter_t iter);
      ~BlockIterator();

      Block next();

      bool has_next();

    private:
      table_iter_t m_iterator;

    };

  TableView();
  TableView( table_t* table );
  TableView(const TableView& other) = default;
  ~TableView() = default;

  TableView& operator=(const TableView& other) = default;

  /**
   * @brief Clears the table
   */
  void 
  clear();

  /**
   * @brief Gets the component with the given id
   *
   * @param id The id of the component to get
   *
   * @return Returns a pointer to the component. Returns nullptr if the component
   * does not exist in the table
   */
  TComponent* 
  get_component(entity_id_t id) const ;

  /**
   * @brief Cre 
   *
   * @tparam TComponent
   * @tparam typename...Args
   * @param id
   * @param ...args
   */
  template<typename...Args>
  TComponent* 
  insert_component(entity_id_t id, 
                   Args&&...args);

  /**
   * @brief Drops the component with the given id
   *
   * @param id
   */
  void 
  remove_component(entity_id_t id);

  /**
   * @brief Enables an component of the table, only it it exists 
   *
   * @param id The id of the component to enable 
   */
  void 
  enable_component(entity_id_t id);

  /**
   * @brief Disables an component of the table
   *
   * @param id The if of the component to disable 
   */
  void 
  disable_component(entity_id_t id);

  /**
   * @brief Tells if an component is enabled or not
   *
   * @param id The component to test if it is enabled or not
   *
   * @return True if the component is enabled. False if it is not 
   */
  bool 
  is_enabled(entity_id_t id);


  /**
   * @brief Gets the size of the table
   *
   * @return Returns the size of the table
   */
  size_t
  size() const;

  /**
   * @brief Gets an iterator of the table
   *
   * @return A new iterator of the table. 
   */
  BlockIterator 
  iterator();

  /**
   * @brief Gets an iterator of the table with an specific chunk_size, offset
   * and stride
   *
   * @return A new iterator of the table. 
   */
  BlockIterator 
  iterator(uint32_t chunk_size, 
           uint32_t offset,
           uint32_t stride);


  /**
   * \brief Gets the pointer to the raw table of this table view
   *
   * \return The pointer to the raw table.
   */
  table_t*
  get_raw();

private:
  table_t*  p_table;
};
  
} /* furious */ 

#include "table_view.inl"

#endif /* ifndef _FURIOUS_TABLE_VIEW_H_ */

