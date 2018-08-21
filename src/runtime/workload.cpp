
#include "workload.h"
#include <cmath>
//#include "execution_plan.h"

namespace furious {

/*
ScopeModifier::ScopeModifier(Workload* workload, System* system) :
  p_workload{workload},
  p_system{system} {

  }

ScopeModifier& ScopeModifier::restrict_to(const std::vector<std::string>& tags) {
  for(auto& system_info : p_workload->m_systems) {
    if(system_info.p_system == p_system) {
      for(auto& tag : tags) {
        system_info.m_tags.push_back(tag);
      }
    }
  }
  return *this;
}

SystemExecInfo::SystemExecInfo(System* system) : p_system(system) {
}
*/

Workload::Workload() :
  m_next_id{0} {
}

Workload::~Workload() {
  for (auto system_info : m_systems) {
    delete system_info.p_system;
  }
}


/*
void Workload::run(float delta_time, Database* database) {
  
  Context context{delta_time, database};
  for(auto& system_info : m_systems) {

    System* const system = system_info.p_system;
    if(system_info.m_tags.size() == 0) {
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

      std::vector<const bitset*> bitsets;
      size_t max_size = 0;
      for(auto tag : system_info.m_tags) {
        auto tagged_entities = database->get_tagged_entities(tag);
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
          Table* table = database->find_table(com_descriptor.m_name);
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
*/

const std::vector<SystemExecInfo>& Workload::get_systems() const {
  return m_systems;
}

} /* furious */ 
