
#include "workload.h"
#include "data/bitset.h" 
#include <cmath>

namespace furious {

ScopeModifier::ScopeModifier(Workload* workload, System* system) :
  p_workload{workload},
  p_system{system} {

  }

ScopeModifier& ScopeModifier::restrict_to(const std::vector<std::string>& tags) {
  for(auto& info : p_workload->m_systems) {
    if(info.second.p_system == p_system) {
      for(auto& tag : tags) {
        info.second.m_tags.push_back(tag);
      }
    }
  }
  return *this;
}

SystemExecInfo::SystemExecInfo(System* system) : p_system(system) {
}

Workload::Workload() {
}

Workload::~Workload() {
  for (auto entry : m_systems) {
    delete entry.second.p_system;
  }
}


void Workload::run(float delta_time, Database* database) {
  
  Context context{delta_time, database};
  for(auto entry : m_systems) {

    System* const system = entry.second.p_system;
    if(entry.second.m_tags.size() == 0) {
      size_t max_block_count=0;
      std::vector<Table*> tables;
      for(auto com_descriptor : system->components()) {
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
          system->apply_block(&context,blocks);
        }
      }
    }
    else {

      std::vector<std::reference_wrapper<const bitset>> bitsets;
      size_t max_size = 0;
      for(auto tag : entry.second.m_tags) {
        auto tagged_entities = database->get_tagged_entities(tag);
        if(tagged_entities) {
          bitsets.emplace_back(*tagged_entities);
          if(tagged_entities.get().size() > max_size) {
            max_size = tagged_entities.get().size();
          }
        }
      }

      bitset entities;
      entities.resize(max_size);
      entities.set();
      for(auto& tagged_entities : bitsets) {
        entities &= tagged_entities;
      }

      std::vector<Table*> tables;
      for(auto com_descriptor : system->components()) {
          Table* table = database->find_table(com_descriptor.m_name);
          tables.push_back(table);
      }

      std::vector<void*> components;
      size_t index = entities.find_first();
      while(index != bitset::npos) {
        /* do something */
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
