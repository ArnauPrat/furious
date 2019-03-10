

#include "transforms.h"
#include "execution_plan.h"
#include "fcc_context.h"

namespace furious {

FccOperator* 
create_subplan(const FccMatch* match)
{
  FccOperator* root = nullptr;
  if(match->m_system.m_component_types.size() > 1)
  {
    // We need to build joins
   /* int32_t size = match->m_system.m_component_types.size();
    FccOperator* left = new Scan(match->m_system.m_component_types[size-2]);
    FccOperator* right = new Scan(match->m_system.m_component_types[size-1]);
    root = new Join(left, right);
    for(int32_t i = size-3; 
        i >= 0; 
        --i)
    {
      FccOperator* left = new Scan(match->m_system.m_component_types[i]);
      root = new Join(left, root);
    }
    */


    int32_t size = match->p_entity_matches.size();
    // First entity match
    
    for(int32_t i = size - 1; i >= 0; --i)
    {
      FccEntityMatch* entity_match = match->p_entity_matches[i];
      uint32_t num_components = entity_match->m_basic_component_types.size();
      for(uint32_t j = 0; j < num_components; ++j)
      {
        if(root == nullptr)
        {
          root = new Scan(entity_match->m_basic_component_types[j]);
        }
        else 
        {
          FccOperator* right = new Scan(entity_match->m_basic_component_types[j]);
          if(entity_match->m_from_expand)
          {
            FccOperator* ref_scan = new Scan(entity_match->m_ref_name);
            right = new Gather(ref_scan, right);
          }
          root = new Join(root, right);
        }
      }
    }
  } 
  else 
  {
    // No join is needed, a single component needs to be scanned
    root = new Scan(match->m_system.m_component_types[0]);
  }

  // Create without Tag Filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_not_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         match->p_entity_matches[0]->m_has_not_tags[i],
                         FccFilterOpType::E_HAS_NOT);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         match->p_entity_matches[0]->m_has_tags[i],
                         FccFilterOpType::E_HAS);
  }

  // Create with components filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_not_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               match->p_entity_matches[0]->m_has_not_components[i],
                               FccFilterOpType::E_HAS_NOT);
  }

  // Create with components filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->m_has_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               match->p_entity_matches[0]->m_has_components[i],
                               FccFilterOpType::E_HAS);
  }

  // Create predicate filters
  for(uint32_t i = 0; i < match->p_entity_matches[0]->p_filter_func.size(); ++i)
  {
    root = new PredicateFilter(root, 
                               match->p_entity_matches[0]->p_filter_func[i]);
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
      root = new Foreach(root, arr);
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
