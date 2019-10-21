
#include "operator.h"
#include "../driver.h"

#include <string.h>

namespace furious 
{

static uint32_t
create_operator(fcc_subplan_t* subplan,
                fcc_operator_type_t operator_type,
                const char* name)
{
  subplan->m_nodes.append(fcc_operator_t());
  uint32_t id = subplan->m_nodes.size()-1;
  fcc_operator_t* op = &subplan->m_nodes[id];
  op->p_subplan = subplan;
  op->m_id = id;
  op->m_type = operator_type;
  op->m_parent = _FURIOUS_COMPILER_INVALID_ID;
  FURIOUS_COPY_AND_CHECK_STR(op->m_name, name, _FURIOUS_COMPILER_MAX_OPERATOR_NAME);
  return id;
}


uint32_t 
create_scan(fcc_subplan_t* subplan, 
            fcc_type_t component,
            fcc_access_mode_t access_mode)
{

  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_SCAN, 
                                "Scan");

  fcc_operator_t* op = &subplan->m_nodes[id];
  fcc_column_t column;
  column.m_type = fcc_column_type_t::E_COMPONENT;
  column.m_component_type = component;
  column.m_access_mode = access_mode;
  op->m_columns.append(column);
  return id;
}

uint32_t 
create_scan(fcc_subplan_t* subplan, 
            const char* ref_name)
{

  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_SCAN, 
                                "Scan");

  fcc_operator_t* op = &subplan->m_nodes[id];
  fcc_column_t column;
  column.m_type = fcc_column_type_t::E_REFERENCE;
  FURIOUS_COPY_AND_CHECK_STR(column.m_ref_name, ref_name, MAX_REF_NAME);
  column.m_access_mode = fcc_access_mode_t::E_READ;
  op->m_columns.append(column);
  return id;
}

uint32_t 
create_fetch(fcc_subplan_t* subplan, 
             fcc_type_t global_type,
             fcc_access_mode_t access_mode)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_FETCH, 
                                "Fetch");

  fcc_operator_t* op = &subplan->m_nodes[id];
  fcc_column_t column;
  column.m_access_mode = access_mode;
  column.m_component_type = global_type;
  column.m_type = fcc_column_type_t::E_GLOBAL;
  op->m_columns.append(column);
  return id;
}

uint32_t 
create_join(fcc_subplan_t* subplan,
            uint32_t left,
            uint32_t right)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_JOIN, 
                                "Join");

  fcc_operator_t* op = &subplan->m_nodes[id];

  subplan->m_nodes[left].m_parent = id;
  subplan->m_nodes[right].m_parent = id;
  op->m_columns.append(subplan->m_nodes[left].m_columns);
  op->m_columns.append(subplan->m_nodes[right].m_columns);
  op->m_join.m_left = left;
  op->m_join.m_right = right;
  return id;
}

uint32_t 
create_leftfilter_join(fcc_subplan_t* subplan,
                       uint32_t left,
                       uint32_t right)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_LEFT_FILTER_JOIN, 
                                "LeftFilterJoin");

  fcc_operator_t* op = &subplan->m_nodes[id];

  subplan->m_nodes[left].m_parent = id;
  subplan->m_nodes[right].m_parent = id;
  op->m_columns.append(subplan->m_nodes[left].m_columns);
  op->m_leftfilter_join.m_left = left;
  op->m_leftfilter_join.m_right = right;
  return id;
}

uint32_t 
create_cross_join(fcc_subplan_t* subplan,
                       uint32_t left,
                       uint32_t right)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_CROSS_JOIN, 
                                "CrossJoin");

  fcc_operator_t* op = &subplan->m_nodes[id];

  subplan->m_nodes[left].m_parent = id;
  subplan->m_nodes[right].m_parent = id;
  op->m_columns.append(subplan->m_nodes[left].m_columns);
  op->m_columns.append(subplan->m_nodes[right].m_columns);
  op->m_cross_join.m_left = left;
  op->m_cross_join.m_right = right;
  return id;
}

uint32_t 
create_predicate_filter(fcc_subplan_t* subplan,
                        uint32_t child,
                        fcc_decl_t filter_func)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_PREDICATE_FILTER, 
                                "PredicateFilter");

  fcc_operator_t* op = &subplan->m_nodes[id];
  subplan->m_nodes[child].m_parent = id;
  op->m_predicate_filter.m_child = child;
  op->m_predicate_filter.m_func_decl = filter_func;
  op->m_columns.append(subplan->m_nodes[child].m_columns);
  return id;
}

uint32_t 
create_tag_filter(fcc_subplan_t* subplan,
                  uint32_t child,
                  const char* tag,
                  fcc_filter_op_type_t op_type,
                  bool on_column)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_TAG_FILTER, 
                                "TagFilter");
  fcc_operator_t* op = &subplan->m_nodes[id];
  FURIOUS_COPY_AND_CHECK_STR(op->m_tag_filter.m_tag, tag, MAX_TAG_NAME);
  subplan->m_nodes[child].m_parent = id;
  op->m_tag_filter.m_child = child;
  op->m_tag_filter.m_op_type = op_type;
  op->m_tag_filter.m_on_column = on_column;
  op->m_columns.append(subplan->m_nodes[child].m_columns);
  return id;
}

uint32_t 
create_component_filter(fcc_subplan_t* subplan,
                        uint32_t child,
                        fcc_type_t component,
                        fcc_filter_op_type_t op_type,
                        bool on_column)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_COMPONENT_FILTER, 
                                "ComponentFilter");
  fcc_operator_t* op = &subplan->m_nodes[id];
  op->m_component_filter.m_filter_type = component;
  subplan->m_nodes[child].m_parent = id;
  op->m_component_filter.m_child = child;
  op->m_component_filter.m_op_type = op_type;
  op->m_component_filter.m_on_column = on_column;
  op->m_columns.append(subplan->m_nodes[child].m_columns);
  return id;
}

uint32_t
create_foreach(fcc_subplan_t* subplan,
               uint32_t child,
               const fcc_system_t* system)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_FOREACH, 
                                "Foreach");
  fcc_operator_t* op = &subplan->m_nodes[id];
  op->m_foreach.p_system = system;
  if(child != _FURIOUS_COMPILER_INVALID_ID)
  {
    op->m_foreach.m_child = child;
    subplan->m_nodes[child].m_parent = id;
    op->m_columns.append(subplan->m_nodes[child].m_columns);
  }
  return id;
}

uint32_t
create_gather(fcc_subplan_t* subplan,
              uint32_t child,
              uint32_t ref_table)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_GATHER, 
                                "Gather");
  fcc_operator_t* op = &subplan->m_nodes[id];
  subplan->m_nodes[child].m_parent = id;
  subplan->m_nodes[ref_table].m_parent = id;
  op->m_gather.m_child = child;
  op->m_gather.m_ref_table = ref_table;
  op->m_columns.append(subplan->m_nodes[child].m_columns);
  return id;
}

uint32_t
create_cascading_gather(fcc_subplan_t* subplan,
                        uint32_t child,
                        uint32_t ref_table)
{
  uint32_t id = create_operator(subplan, 
                                fcc_operator_type_t::E_CASCADING_GATHER, 
                                "CascadingGather");
  fcc_operator_t* op = &subplan->m_nodes[id];
  subplan->m_nodes[child].m_parent = id;
  subplan->m_nodes[ref_table].m_parent = id;
  op->m_cascading_gather.m_child = child;
  op->m_cascading_gather.m_ref_table = ref_table;
  op->m_columns.append(subplan->m_nodes[child].m_columns);

  return id;
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

uint32_t 
apply_predicate_filters(fcc_subplan_t* subplan,
                        const fcc_stmt_t* match,
                        const fcc_entity_match_t* entity_match,
                        uint32_t root)
{
  uint32_t local_root = root;

  // Create predicate filters
  for(uint32_t i = 0; i < entity_match->m_filter_func.size(); ++i)
  {
    local_root = create_predicate_filter(subplan,
                                         local_root, 
                                         entity_match->m_filter_func[i]);
  }
  return local_root;
}

uint32_t
apply_filters(fcc_subplan_t* subplan,
              const fcc_stmt_t* match, 
              const fcc_entity_match_t* entity_match, 
              uint32_t root)
{

  uint32_t local_root = root;

  // Create without Tag Filters
  for(uint32_t i = 0; i < entity_match->m_has_not_tags.size(); ++i)
  {
    local_root = create_tag_filter(subplan,
                                   local_root, 
                                   entity_match->m_has_not_tags[i],
                                   fcc_filter_op_type_t::E_HAS_NOT);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < entity_match->m_has_tags.size(); ++i)
  {
    local_root = create_tag_filter(subplan,
                                   local_root, 
                                   entity_match->m_has_tags[i],
                                   fcc_filter_op_type_t::E_HAS);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_not_components.size(); ++i)
  {
    local_root = create_component_filter(subplan,
                                         local_root, 
                                         entity_match->m_has_not_components[i],
                                         fcc_filter_op_type_t::E_HAS_NOT);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_components.size(); ++i)
  {
    local_root = create_component_filter(subplan,
                                         local_root, 
                                         entity_match->m_has_components[i],
                                         fcc_filter_op_type_t::E_HAS);
  }

  return local_root;
}

uint32_t
apply_filters_reference(fcc_subplan_t* subplan,
                        const fcc_stmt_t* match, 
                        const fcc_entity_match_t* entity_match, 
                        uint32_t root)
{

  uint32_t local_root = root;

  // Create without Tag Filters
  for(uint32_t i = 0; i < entity_match->m_has_not_tags.size(); ++i)
  {
    local_root = create_tag_filter(subplan,
                                   local_root, 
                                   entity_match->m_has_not_tags[i],
                                   fcc_filter_op_type_t::E_HAS_NOT,
                                   true);
  }

  // Create with tag filters
  for(uint32_t i = 0; i < entity_match->m_has_tags.size(); ++i)
  {
    local_root = create_tag_filter(subplan,
                                   local_root, 
                                   entity_match->m_has_tags[i],
                                   fcc_filter_op_type_t::E_HAS,
                                   true);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_not_components.size(); ++i)
  {
    local_root = create_component_filter(subplan,
                                         local_root, 
                                         entity_match->m_has_not_components[i],
                                         fcc_filter_op_type_t::E_HAS_NOT, 
                                         true);
  }

  // Create with components filters
  for(uint32_t i = 0; i < entity_match->m_has_components.size(); ++i)
  {
    local_root = create_component_filter(subplan,
                                         local_root, 
                                         entity_match->m_has_components[i],
                                         fcc_filter_op_type_t::E_HAS, 
                                         true);
  }

  return local_root;
}
void 
subplan_init(const fcc_stmt_t* match,
             fcc_subplan_t* subplan)
{
  *subplan = {0};
  uint32_t root = _FURIOUS_COMPILER_INVALID_ID;
  subplan->m_requires_sync = false;

  int32_t size = match->p_entity_matches.size();
  for(int32_t i = size - 1; i >= 0; --i)
  {
    fcc_entity_match_t* entity_match = match->p_entity_matches[i];
    uint32_t num_components = entity_match->m_component_types.size();
    uint32_t local_root = _FURIOUS_COMPILER_INVALID_ID;
    for(uint32_t j = 0; j < num_components; ++j)
    {
      fcc_component_match_t* match_type = &entity_match->m_component_types[j];

      fcc_access_mode_t access_mode = match_type->m_is_read_only ? fcc_access_mode_t::E_READ : fcc_access_mode_t::E_READ_WRITE;
      if(local_root == _FURIOUS_COMPILER_INVALID_ID)
      {
        if(match_type->m_is_global)
        {
          local_root = create_fetch(subplan,
                                    entity_match->m_component_types[j].m_type, 
                                    access_mode);
        }
        else
        {
          local_root = create_scan(subplan,
                                   entity_match->m_component_types[j].m_type, 
                                   access_mode);
        }
      }
      else 
      {
        uint32_t right = _FURIOUS_COMPILER_INVALID_ID;
        if(match_type->m_is_global)
        {
          right = create_fetch(subplan,
                               entity_match->m_component_types[j].m_type, 
                               access_mode);

        }
        else
        {
          right = create_scan(subplan,
                              entity_match->m_component_types[j].m_type, 
                              access_mode);

        }

        if (subplan->m_nodes[local_root].m_type == fcc_operator_type_t::E_FETCH || 
            subplan->m_nodes[right].m_type == fcc_operator_type_t::E_FETCH)
        {
          local_root = create_cross_join(subplan,
                                         local_root, 
                                         right);
        }
        else
        {
          local_root = create_join(subplan,
                                   local_root, 
                                   right);
        }

      }
    }

    if(local_root != _FURIOUS_COMPILER_INVALID_ID)
    {
      local_root = apply_filters(subplan,
                                 match, 
                                 entity_match, 
                                 local_root);
    }

    bool non_component_expand = local_root == _FURIOUS_COMPILER_INVALID_ID; 
    if(entity_match->m_from_expand)
    {
      uint32_t ref_scan = create_scan(subplan,
                                      entity_match->m_ref_name);

      if(!non_component_expand)
      {
        bool cascading = false;
        for(uint32_t j = 0; j < num_components; ++j)
        {
          fcc_type_t expand_type = entity_match->m_component_types[j].m_type;
          uint32_t num_match_components = match->p_entity_matches[match->p_entity_matches.size()-1]->m_component_types.size();
          for(uint32_t k = 0; k < num_match_components; ++k)
          {
            fcc_type_t match_type = match->p_entity_matches[match->p_entity_matches.size()-1]->m_component_types[k].m_type;

            char expand_type_name[MAX_TYPE_NAME];
            fcc_type_name(expand_type, 
                          expand_type_name,
                          MAX_TYPE_NAME);

            char match_type_name[MAX_TYPE_NAME];
            fcc_type_name(match_type, 
                          match_type_name,
                          MAX_TYPE_NAME);

            if(!(fcc_type_access_mode(match_type) == fcc_access_mode_t::E_READ) &&
               strcmp(expand_type_name, match_type_name) == 0)
            {
              cascading = true;
              break;
            }
          }
        }

        if(cascading)
        {
          local_root = create_cascading_gather(subplan,
                                               local_root,
                                               ref_scan);
          subplan->m_requires_sync = true;
        }
        else
        {
          local_root = create_gather(subplan,
                                     local_root,
                                     ref_scan);
        }
      }
      else
      {
        local_root = apply_filters_reference(subplan,
                                             match, 
                                             entity_match, 
                                             ref_scan);
      }
    }

    if(root == _FURIOUS_COMPILER_INVALID_ID)
    {
      root = local_root;
    }
    else
    {
      if(non_component_expand)
      {
        root = create_leftfilter_join(subplan,
                                      root, 
                                      local_root);
      }
      else
      {
        root = create_join(subplan,
                           root, 
                           local_root);
      }
    }

    root = apply_predicate_filters(subplan, 
                                   match, 
                                   entity_match, 
                                   root);
  }


  // Create Operation operator
  switch(match->m_operation_type)
  {
    case fcc_operation_type_t::E_UNKNOWN:
      assert(false && "Operatoion Type is not set");
      break;
    case fcc_operation_type_t::E_FOREACH:
      // * Read basic coponent types and create one scan for each of them.
      // * Create joins to create the final table to the system will operate on
      // * Create filters for with_component, without_component, with_tag,
      //  without_tag and filter predicate.
      // * Create Foreach operator
      root = create_foreach(subplan,
                            root, 
                            match->p_system);
      break;
  };

  subplan->m_root = root;
}

void 
subplan_release(fcc_subplan_t* subplan)
{
}

}
