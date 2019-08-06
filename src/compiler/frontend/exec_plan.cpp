
#include "frontend/transforms.h"
#include "execution_plan.h"
#include "fcc_context.h"
#include "clang_tools.h"
#include "operator.h"

namespace furious 
{

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


FccOperator* 
apply_predicate_filters(const FccMatch* match,
                        const FccEntityMatch* entity_match,
                        FccOperator* root)
{
  FccOperator* local_root = root;

  // Create predicate filters
  for(uint32_t i = 0; i < entity_match->p_filter_func.size(); ++i)
  {
    local_root = new PredicateFilter(local_root, 
                                     entity_match->p_filter_func[i]);
  }
  return local_root;
}

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

void 
init_subplan(const FccMatch* match,
             FccSubPlan* subplan)
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

    root = apply_predicate_filters(match, entity_match, root);
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

  subplan->p_root = root;
  subplan->p_ast = match->p_ast_context;
}

void 
release_subplan(FccSubPlan* subplan)
{
  delete subplan->p_root;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static bool
is_dependent(const FccExecPlan* exec_plan, 
             uint32_t node_a, 
             uint32_t node_b)
{
  DynArray<QualType> write_types_b;

  const FccMatch* match_b = exec_plan->p_matches[node_b];
  const DynArray<FccMatchType>& match_types_b = match_b->p_system->m_match_types;
  uint32_t size_b = match_types_b.size(); 
  for(uint32_t i = 0; i < size_b; ++i)
  {
    const QualType& type = match_types_b[i].m_type;
    bool is_read_only = match_types_b[i].m_is_read_only;
    if(!is_read_only)
    {
      write_types_b.append(type);
    }
  }

  const FccMatch* match_a = exec_plan->p_matches[node_a];
  const DynArray<FccMatchType>& match_types_a = match_a->p_system->m_match_types;
  uint32_t size_a = match_types_a.size();
  uint32_t priority_a = match_a->m_priority;
  for(uint32_t i = 0; i < size_a; ++i)
  {
    const QualType& type_a = match_types_a[i].m_type;
    std::string name_a = get_type_name(type_a);
    bool is_read_only = match_types_a[i].m_is_read_only;

    size_b = write_types_b.size();
    uint32_t priority_b = match_b->m_priority;
    for(uint32_t ii = 0; ii < size_b; ++ii)
    {
      const QualType& type_b = write_types_b[ii];
      std::string name_b = get_type_name(type_b);
      if(name_a == name_b && is_read_only)
      {
        return true;
      }

      if(name_a == name_b && 
         priority_a >= priority_b)
      {
        return true;
      }
    }
  }
  return false;
}

void
insert(FccExecPlan* exec_plan, 
       const FccMatch* exec_info)
{
  exec_plan->m_nodes.append(FccExecPlanNode());
  exec_plan->p_matches.append(exec_info);
  exec_plan->m_subplans.append({nullptr, nullptr});

  // Adding for dependencies (if any)
  uint32_t num_nodes = exec_plan->m_nodes.size();
  uint32_t added_node_id = num_nodes -1; 
  FccExecPlanNode* added_node = &exec_plan->m_nodes[added_node_id];
  const FccMatch* added_node_match = exec_plan->p_matches[added_node_id];

  init_subplan(added_node_match, 
               &exec_plan->m_subplans[added_node_id]);

  for(uint32_t i = 0; 
      i < num_nodes - 1; 
      ++i)
  {
    uint32_t next_node_id = i;
    FccExecPlanNode* next_node = &exec_plan->m_nodes[next_node_id];
    const FccMatch* next_node_match = exec_plan->p_matches[next_node_id];
    if(is_dependent(exec_plan, added_node_id, next_node_id))
    {
      next_node->m_children.append(added_node_id);
      added_node->m_parents.append(next_node_id);
      printf("%s depends on %s \n", 
             get_type_name(added_node_match->p_system->m_system_type).c_str(),
             get_type_name(next_node_match->p_system->m_system_type).c_str()
             );
    }

    if(is_dependent(exec_plan, next_node_id, added_node_id))
    {
      added_node->m_children.append(next_node_id);
      next_node->m_parents.append(added_node_id);
      printf("%s depends on %s \n", 
             get_type_name(next_node_match->p_system->m_system_type).c_str(),
             get_type_name(added_node_match->p_system->m_system_type).c_str()
             );
    }
  }

  // Re-evaluating roots
  exec_plan->m_roots.clear();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(exec_plan->m_nodes[i].m_parents.size() == 0)
    {
      exec_plan->m_roots.append(i);
    }
  }

  assert(exec_plan->m_nodes.size() == exec_plan->p_matches.size() &&
         exec_plan->m_nodes.size() == exec_plan->m_subplans.size() &&
         "Inconsistent number of node attributes in ExecutionPlan");
}

void
dfs(const FccExecPlan* exec_plan, 
    bool* visited,
    uint32_t* departure,
    uint32_t* time,
    uint32_t node_id)
{
  visited[node_id] = true;
  const FccExecPlanNode* node = &exec_plan->m_nodes[node_id];
  const uint32_t size = node->m_children.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const uint32_t child_id = node->m_children[i];
    if(!visited[child_id])
    {
      dfs(exec_plan,
          visited,
          departure,
          time,
          child_id);

    }
  }

  departure[node_id] = *time;
  ++(*time);
}

bool
is_acyclic(const FccExecPlan* exec_plan)
{
  uint32_t num_nodes = exec_plan->m_nodes.size();
  bool visited[num_nodes];
  memset(visited, 0, sizeof(bool)*num_nodes);
  uint32_t departure[num_nodes];
  memset(departure, 0, sizeof(bool)*num_nodes);

  uint32_t time=0;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    if(!visited[i] && exec_plan->m_nodes[i].m_parents.size() == 0)
    {
      dfs(exec_plan, 
          visited, 
          departure, 
          &time, 
          i);
    }
  }

  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    uint32_t node_id = i;
    const FccExecPlanNode* node = &exec_plan->m_nodes[node_id];
    const uint32_t num_children = node->m_children.size();
    for(uint32_t ii = 0; ii < num_children; ++ii)
    {
      const uint32_t child_id = node->m_children[ii];
      if(departure[node_id] <= departure[child_id])
      {
        return false;
      }
    }
  }
  return true;
}


static bool
all_visited_parents(const FccExecPlan* exec_plan, 
                    uint32_t node_id,
                    bool * visited)
{
  const DynArray<uint32_t> parents = exec_plan->m_nodes[node_id].m_parents;
  const uint32_t num_elements = parents.size();
  for(uint32_t i = 0; i < num_elements; ++i)
  {
    if(!visited[parents[i]])
    {
      return false;
    }
  }
  return true;
}

DynArray<uint32_t>
get_valid_exec_sequence(const FccExecPlan* exec_plan)
{
  DynArray<uint32_t> ret;
  const uint32_t num_nodes = exec_plan->m_nodes.size();
  bool visited_nodes[num_nodes];
  memset(visited_nodes, 0, sizeof(bool)*num_nodes);
  bool found = true;
  while(found)
  {
    found = false;
    for (uint32_t i = 0; i < num_nodes; ++i) 
    {
      if(!visited_nodes[i] && all_visited_parents(exec_plan, i, visited_nodes))
      {
        found = true;
        DynArray<uint32_t> next_frontier;
        DynArray<uint32_t> current_frontier;
        current_frontier.append(i);
        visited_nodes[i] = true;
        while(current_frontier.size() > 0)
        {
          const uint32_t frontier_size = current_frontier.size();
          for(uint32_t ii = 0; ii < frontier_size; ++ii)
          {
            const uint32_t next_node_id = current_frontier[ii];
            ret.append(next_node_id);
            const FccExecPlanNode* next_node = &exec_plan->m_nodes[next_node_id]; 
            const DynArray<uint32_t>& children = next_node->m_children;
            const uint32_t num_children = children.size();
            for(uint32_t j = 0; j < num_children; ++j)
            {
              const uint32_t child_id = children[j];
              if(!visited_nodes[child_id] && all_visited_parents(exec_plan, child_id, visited_nodes))
              {
                next_frontier.append(child_id);
                visited_nodes[child_id] = true;
              }
            }
          }
          current_frontier = next_frontier;
          next_frontier.clear();
        }
      }
    }
  }
  return ret;
}

FccCompilationErrorType
create_execplan(const FccMatch* matches[], 
                   uint32_t num_matches,
                   FccExecPlan** exec_plan)
{
  *exec_plan = nullptr;
  FccExecPlan* ret_exec_plan = new FccExecPlan();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
      insert(ret_exec_plan, matches[i]);
  }

  if(!is_acyclic(ret_exec_plan)) 
  {
    return FccCompilationErrorType::E_CYCLIC_DEPENDENCY_GRAPH;
  }

  *exec_plan = ret_exec_plan;
  return FccCompilationErrorType::E_NO_ERROR;
}

void
destroy_execplan(FccExecPlan* exec_plan)
{
  uint32_t num_nodes = exec_plan->m_subplans.size();
  for(uint32_t i = 0; 
      i < num_nodes; 
      ++i)
  {
    release_subplan(&exec_plan->m_subplans[i]);
  }
  delete exec_plan;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
FccSubPlanVisitor::traverse(const FccSubPlan* subplan)
{
    subplan->p_root->accept(this);
}


} /* furious */ 
