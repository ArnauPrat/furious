
#include "workload.h"
#include "data/bitset.h" 
#include <cmath>

namespace furious {

Workload::Workload() {
}

Workload::~Workload() {
  for (auto entry : m_systems) {
    delete entry.second;
  }
}


void Workload::run(float delta_time, Database* database) {
  
  Context context{delta_time, database};
  for(auto system : m_systems) {

    size_t max_block_count=0;
    std::vector<Table*> tables;
    for(auto com_descriptor : system.second->components()) {
      Table* table = database->find_table(com_descriptor.m_name);
      tables.push_back(table);
      max_block_count = std::max(max_block_count, table->get_blocks_mask().size());
    }

    bitset matching_blocks{max_block_count};
    matching_blocks.set();
    for(auto table : tables) {
      matching_blocks &= table->get_blocks_mask();
    }

    std::vector<TBlock*> blocks;
    blocks.resize(tables.size(), nullptr);
    for(size_t i = 0; i < matching_blocks.size(); ++i) {
      if(matching_blocks[i]) {
        size_t j = 0;
        for(auto table : tables) {
          blocks[j] = table->get_block(i);
          ++j;
        }
        system.second->apply_block(&context,blocks);
      }
    }

  }
}

} /* furious */ 
