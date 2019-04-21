

#include "transforms.h"
#include "execution_plan.h"
#include "fcc_context.h"
#include "clang_tools.h"

namespace furious 
{

FccOperator*
apply_filters(const FccMatch* match, 
              const FccEntityMatch* entity_match, 
              FccOperator* root)
{

  FccOperator* local_root = root;

  // Create without Tag Filters
  for(uint32_t i = 0; i < entity_match->m_has_not_tags.size(); ++i)
  {
    local_root = new TagFilter(local_root, 
                               entity_match->m_has_not_tags[i],
                               FccFilterOpType::E_HAS_NOT);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < entity_match->m_has_tags.size(); ++i)
  {
    local_root = new TagFilter(local_root, 
                               entity_match->m_has_tags[i],
                               FccFilterOpType::E_HAS);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_not_components.size(); ++i)
  {
    local_root = new ComponentFilter(local_root, 
                                     entity_match->m_has_not_components[i],
                                     FccFilterOpType::E_HAS_NOT);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_components.size(); ++i)
  {
    local_root = new ComponentFilter(local_root, 
                                     entity_match->m_has_components[i],
                                     FccFilterOpType::E_HAS);
  }

  // Create predicate filters
  for(uint32_t i = 0; i < entity_match->p_filter_func.size(); ++i)
  {
    local_root = new PredicateFilter(local_root, 
                                     entity_match->p_filter_func[i]);
  }
  return local_root;
}

FccOperator*
apply_filters_reference(const FccMatch* match, 
                        const FccEntityMatch* entity_match, 
                        FccOperator* root)
{

  FccOperator* local_root = root;

  // Create without Tag Filters
  for(uint32_t i = 0; i < entity_match->m_has_not_tags.size(); ++i)
  {
    local_root = new TagFilter(local_root, 
                               entity_match->m_has_not_tags[i],
                               FccFilterOpType::E_HAS_NOT,
                               true);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < entity_match->m_has_tags.size(); ++i)
  {
    local_root = new TagFilter(local_root, 
                               entity_match->m_has_tags[i],
                               FccFilterOpType::E_HAS,
                               true);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_not_components.size(); ++i)
  {
    local_root = new ComponentFilter(local_root, 
                                     entity_match->m_has_not_components[i],
                                     FccFilterOpType::E_HAS_NOT, 
                                     true);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_components.size(); ++i)
  {
    local_root = new ComponentFilter(local_root, 
                                     entity_match->m_has_components[i],
                                     FccFilterOpType::E_HAS, 
                                     true);
  }

  return local_root;
}

FccOperator* 
create_subplan(const FccMatch* match)
{
  FccOperator* root = nullptr;

  int32_t size = match->p_entity_matches.size();
  for(int32_t i = size - 1; i >= 0; --i)
  {
    FccEntityMatch* entity_match = match->p_entity_matches[i];
    uint32_t num_components = entity_match->m_match_types.size();
    FccOperator* local_root = nullptr;
    for(uint32_t j = 0; j < num_components; ++j)
    {
      FccMatchType* match_type = &entity_match->m_match_types[j];

      FccAccessMode access_mode = match_type->m_is_read_only ? FccAccessMode::E_READ : FccAccessMode::E_READ_WRITE;
      if(local_root == nullptr)
      {
        if(match_type->m_is_global)
        {
          local_root = new Fetch(entity_match->m_match_types[j].m_type, 
                                 access_mode);
        }
        else
        {
          local_root = new Scan(entity_match->m_match_types[j].m_type, 
                                access_mode);
        }
      }
      else 
      {
        FccOperator* right = nullptr;
        if(match_type->m_is_global)
        {
          right = new Fetch(entity_match->m_match_types[j].m_type, 
                                 access_mode);

          local_root = new CrossJoin(local_root, 
                                     right);
        }
        else
        {
          right = new Scan(entity_match->m_match_types[j].m_type, 
                           access_mode);

          local_root = new Join(local_root, 
                                right);
        }

      }
    }

    if(local_root != nullptr)
    {
      local_root = apply_filters(match, entity_match, local_root);
    }

    bool non_component_expand = local_root == nullptr; 
    if(entity_match->m_from_expand)
    {
      FccOperator* ref_scan = new Scan(entity_match->m_ref_name);

      if(!non_component_expand)
      {
        bool cascading = false;
        for(uint32_t j = 0; j < num_components; ++j)
        {
          QualType expand_type = entity_match->m_match_types[j].m_type;
          uint32_t num_match_components = match->p_entity_matches[match->p_entity_matches.size()-1]->m_match_types.size();
          for(uint32_t k = 0; j < num_match_components; ++j)
          {
            QualType match_type = match->p_entity_matches[match->p_entity_matches.size()-1]->m_match_types[k].m_type;
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
                                           local_root);
        }
        else
        {
          local_root = new Gather(ref_scan, 
                                  local_root);
        }
      }
      else
      {
        local_root = apply_filters_reference(match, entity_match, ref_scan);
      }
    }

    if(root == nullptr)
    {
      root = local_root;
    }
    else
    {
      if(non_component_expand)
      {
        root = new LeftFilterJoin(root, 
                                  local_root);
      }
      else
      {
        root = new Join(root, 
                        local_root);
      }
    }
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
      arr.append(match->p_system);
      root = new Foreach(root, 
                         arr);
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
merge_foreach(const Foreach* foreach1, 
              const Foreach* foreach2) {
  return nullptr;
}

} /* furious */ 
