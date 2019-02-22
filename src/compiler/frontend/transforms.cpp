

#include "transforms.h"
#include "execution_plan.h"
#include "fcc_context.h"

namespace furious {

FccOperator* 
create_subplan(const FccExecInfo* exec_info)
{
  FccOperator* root = nullptr;
  if(exec_info->m_basic_component_types.size() > 1)
  {
    // We need to build joins
    int32_t size = exec_info->m_basic_component_types.size();
    FccOperator* left = new Scan(exec_info->m_basic_component_types[size-2]);
    FccOperator* right = new Scan(exec_info->m_basic_component_types[size-1]);
    root = new Join(left, right);
    for(int32_t i = size-3; 
        i >= 0; 
        --i)
    {
      FccOperator* left = new Scan(exec_info->m_basic_component_types[i]);
      root = new Join(left, root);
    }
  } 
  else 
  {
    // No join is needed, a single component needs to be scanned
    root = new Scan(exec_info->m_basic_component_types[0]);
  }

  // Create without Tag Filters
  for(uint32_t i = 0; i < exec_info->m_has_not_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         exec_info->m_has_not_tags[i],
                         FccFilterOpType::E_HAS_NOT);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < exec_info->m_has_tags.size(); ++i)
  {
    root = new TagFilter(root, 
                         exec_info->m_has_tags[i],
                         FccFilterOpType::E_HAS);
  }

  // Create with components filters
  for(uint32_t i = 0; i < exec_info->m_has_not_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               exec_info->m_has_not_components[i],
                               FccFilterOpType::E_HAS_NOT);
  }

  // Create with components filters
  for(uint32_t i = 0; i < exec_info->m_has_components.size(); ++i)
  {
    root = new ComponentFilter(root, 
                               exec_info->m_has_components[i],
                               FccFilterOpType::E_HAS);
  }

  // Create predicate filters
  for(uint32_t i = 0; i < exec_info->p_filter_func.size(); ++i)
  {
    root = new PredicateFilter(root, 
                               exec_info->p_filter_func[i]);
  }

  // Create Operation operator
  switch(exec_info->m_operation_type)
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
      DynArray<const FccSystemInfo*> arr;
      arr.append(&exec_info->m_system);
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
