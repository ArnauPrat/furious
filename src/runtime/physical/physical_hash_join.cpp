

#include "physical_hash_join.h"
#include <functional>
#include <cassert>

namespace furious {

    static uint32_t size = 1024;

    PhysicalHashJoin::PhysicalHashJoin(IPhysicalOperatorSPtr left, 
                                       IPhysicalOperatorSPtr right) : 
      p_left(left),
      p_right(right),
      m_hash_table(size),
      m_joined_rows(size)
      {}

    PhysicalHashJoin::~PhysicalHashJoin() {
      for (auto row : m_joined_rows) {
        delete row;
      }
    }

    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    BaseRow* PhysicalHashJoin::next() {
      BaseRow* next = p_right->next();
      while(next != nullptr) {
        uint32_t position = get_hash_position(next);
        auto candidates = m_hash_table[position];
        for(auto it = candidates.begin(); it != candidates.end(); ++it ) {
          if((*it)->m_id == next->m_id) {
            Row* joined_row = new Row(next->m_id, (*it), next);
            m_joined_rows.push_back(joined_row);
            return joined_row;
          }
        }
        next = p_right->next();
      }
      return next;
    }

    void PhysicalHashJoin::open() {
      p_left->open();
      BaseRow* next = p_left->next();
      while(next != nullptr){
        m_hash_table[get_hash_position(next)].push_back(next);
        next = p_left->next();
      }
      p_right->open();
    }

    void PhysicalHashJoin::close() {
      p_left->close();
      p_right->close();
    }

    uint32_t PhysicalHashJoin::get_hash_position(BaseRow* row) {
      uint32_t position = std::hash<uint32_t>{}(row->m_id) % size;
      return position;
    }

    uint32_t PhysicalHashJoin::num_children()  const  {
      return 2;
    }

    IPhysicalOperatorSPtr  PhysicalHashJoin::child(uint32_t i ) const {
      assert(i < 2);
      if(i == 0) return p_left;
      return p_right;
    }

    std::string PhysicalHashJoin::str() const  {
      return "PhysicalHashJoin()";
    }
    
} /* furious
 */ 
