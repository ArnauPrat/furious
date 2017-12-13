

#ifndef _FURIOUS_PHYSICAL_HASH_JOIN_H
#define _FURIOUS_PHYSICAL_HASH_JOIN_H 

#include "physical_plan.h"
#include "../table.h"
#include "../common.h"

#include <cassert>
#include <map>

namespace furious  {

class PhysicalHashJoin : public IPhysicalOperator {

public:
  class Row : public BaseRow {
  public:
    Row ( EntityId id, BaseRow* row_left, BaseRow* row_right) : 
      BaseRow(id),
      p_row_left(row_left),
      p_row_right(row_right) {}

    virtual ~Row() = default;

    virtual void* column(uint32_t column) override {
      assert(column < num_columns());
      return column < p_row_left->num_columns() ? p_row_left->column(column) 
                                               : p_row_right->column(right_column_index(column));
    }

    virtual uint32_t column_size( uint32_t column) const override {
      assert(column < num_columns());
      return column < p_row_left->num_columns() ? p_row_left->column_size(column) 
                                               : p_row_right->column_size(right_column_index(column));
    }

    virtual uint32_t num_columns() const override {
      return p_row_left->num_columns() + p_row_right->num_columns();
    }

  private:

    /**
     * @brief Translates the column index of the row to the one of the right row
     *
     * @param column The column index
     *
     * @return The right column index
     */
    uint32_t right_column_index( uint32_t column ) const {
      return column - p_row_left->num_columns();
    }

    BaseRow* p_row_left;
    BaseRow* p_row_right;
  };

public:

  PhysicalHashJoin( IPhysicalOperatorSPtr left, IPhysicalOperatorSPtr right); 
  virtual ~PhysicalHashJoin();

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////

  BaseRow* next() override;

  void  open() override;

  void  close() override;

  virtual uint32_t num_children()  const override ;

  virtual IPhysicalOperatorSPtr  child(uint32_t i) const override;

  virtual std::string str() const override;

private:

  uint32_t get_hash_position(BaseRow* row);

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////

  IPhysicalOperatorSPtr                 p_left;
  IPhysicalOperatorSPtr                 p_right;
  std::vector<std::vector<BaseRow*>>    m_hash_table;
  std::vector<Row*>                     m_joined_rows;
};

} /* physical_hash_join */ 
#endif
