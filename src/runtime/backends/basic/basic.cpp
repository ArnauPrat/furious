


#include "../../workload.h"
#include "../../../data/database.h"
#include "basic.h"

namespace furious {

void Basic::compile(Workload* workload,
             Database* database) {
  p_workload = workload;
  p_database = database;
}

void Basic::run(float delta) {

  Context context{delta, p_database};
  for(auto& system_info : p_workload->get_systems()) {

    System* const system = system_info.p_system;
    if(system_info.m_tags.size() == 0) {
      size_t max_block_count=0;
      std::vector<Table*> tables;
      for(auto com_descriptor : system->components()) {
        Table* table = p_database->find_table(com_descriptor.m_name);
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
          system->apply_block(&context,blocks);
        }
      }
    }
    else {

      std::vector<const bitset*> bitsets;
      size_t max_size = 0;
      for(auto tag : system_info.m_tags) {
        auto tagged_entities = p_database->get_tagged_entities(tag);
        if(tagged_entities) {
          bitsets.emplace_back(*tagged_entities);
          max_size = std::max(tagged_entities.get()->size(), max_size);
        }
      }

      bitset entities;
      entities.resize(max_size);
      entities.set();
      for(auto tagged_entities : bitsets) {
        entities &= *tagged_entities;
      }

      std::vector<Table*> tables;
      for(auto com_descriptor : system->components()) {
          Table* table = p_database->find_table(com_descriptor.m_name);
          tables.push_back(table);
      }

      std::vector<void*> components;
      size_t index = entities.find_first();
      while(index != bitset::npos) {
        components.clear();
        bool qualify = true;

        for(auto table : tables) {
          void* component = table->get_element(index);
          if(component != nullptr) {
            components.push_back(component);
          } else {
            qualify = false;
            break;
          }
        }
        if(qualify) {
          system->apply(&context, index, components);
        }
        index = entities.find_next(index);
      }
    }
  } 
}

  
} /* furious */ 
