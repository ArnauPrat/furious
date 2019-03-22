

#include "transforms.h"
#include "execution_plan.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious 
{

FccOperator* 
create_subplan(const FccMatch* match)
{
  FccOperator* root = nullptr;

  int32_t size = match->p_entity_matches.size();
  for(int32_t i = size - 1; i >= 0; --i)
  {
    FccEntityMatch* entity_match = match->p_entity_matches[i];
    uint32_t num_components = entity_match->m_basic_component_types.size();
    FccOperator* local_root = nullptr;
    for(uint32_t j = 0; j < num_components; ++j)
    {

      FccAccessMode access_mode = get_access_mode(entity_match->m_basic_component_types[j]);
      if(access_mode == FccAccessMode::E_READ_WRITE && match->m_system.m_is_outputwriteonly)
      {
        access_mode = FccAccessMode::E_WRITE;
      }

      if(local_root == nullptr)
      {
        local_root = new Scan(entity_match->m_basic_component_types[j], 
                              access_mode,
                              match->p_fcc_context);
      }
      else 
      {
        FccOperator* right = new Scan(entity_match->m_basic_component_types[j], 
                                      access_mode,
                                      match->p_fcc_context);

        local_root = new Join(local_root, 
                              right, 
                              match->p_fcc_context);
      }
    }

    if(entity_match->m_from_expand)
    {
      FccOperator* ref_scan = new Scan(entity_match->m_ref_name, 
                                       match->p_fcc_context);

      bool cascading = false;
      for(uint32_t j = 0; j < num_components; ++j)
      {
        QualType expand_type = entity_match->m_basic_component_types[j];
        uint32_t num_match_components = match->p_entity_matches[match->p_entity_matches.size()-1]->m_basic_component_types.size();
        for(uint32_t k = 0; j < num_match_components; ++j)
        {
          QualType match_type = match->p_entity_matches[match->p_entity_matches.size()-1]->m_basic_component_types[k];
          if(!(get_access_mode(match_type) == FccAccessMode::E_READ) &&
              get_type_name(expand_type) == get_type_name(match_type))
          {
            cascading = true;
            break;
          }
        }
      }

      if(cascading)
      {
        local_root = new CascadingGather(ref_scan, 
                                         local_root, 
                                         match->p_fcc_context);
      }
      else
      {
        local_root = new Gather(ref_scan, 
                                local_root, 
                                match->p_fcc_context);
      }
    }
    if(root == nullptr)
    {
      root = local_root;
    }
    else
    {
      root = new Join(root, 
                      local_root, 
                      match->p_fcc_context);
    }
  }

  // Create without Tag Filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_not_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         match->p_entity_matches[0]->m_has_not_tags[i],
                         FccFilterOpType::E_HAS_NOT, 
                         match->p_fcc_context);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         match->p_entity_matches[0]->m_has_tags[i],
                         FccFilterOpType::E_HAS, 
                         match->p_fcc_context);
  }

  // Create with components filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_not_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               match->p_entity_matches[0]->m_has_not_components[i],
                               FccFilterOpType::E_HAS_NOT, 
                               match->p_fcc_context);
  }

  // Create with components filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               match->p_entity_matches[0]->m_has_components[i],
                               FccFilterOpType::E_HAS, 
                               match->p_fcc_context);
  }

  // Create predicate filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->p_filter_func.size(); ++i)
  {
    root = new PredicateFilter(root, 
                               match->p_entity_matches[0]->p_filter_func[i], 
                               match->p_fcc_context);
  }

  // Create Operation operator
  switch(match->m_operation_type)
  {
    case FccOperationType::E_UNKNOWN:
      assert(false && "Operatoion Type is not set");
      break;
    case FccOperationType::E_FOREACH:
      // * Read basic coponent types and create one scan for each of them.
      // * Create joins to create the final table to the system will operate on
      // * Create filters for with_component, without_component, with_tag,
      //  without_tag and filter predicate.
      // * Create Foreach operator
      DynArray<const FccSystem*> arr;
      arr.append(&match->m_system);
      root = new Foreach(root, 
                         arr, 
                         match->p_fcc_context);
      break;
  };
  return root;
}

void 
destroy_subplan(FccOperator* root)
{
  delete root;
}

Foreach* 
merge_foreach(FccContext* context,
              const Foreach* foreach1, 
              const Foreach* foreach2) {
  return nullptr;
}
  
} /* furious */ 
