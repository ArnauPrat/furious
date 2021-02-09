

#include "../../common/str_builder.h"
#include "../frontend/operator.h"
#include "backend/codegen.h"
#include "codegen_tools.h"
#include "consumer.h"
#include "fcc_context.h"
#include "producer.h"


void
consume(FILE* fd,
        const fcc_operator_t* op,
        const char* source,
        const fcc_operator_t* caller);

static void
consume_scan(FILE*fd,
             const fcc_operator_t* scan,
             const char* source,
             const fcc_operator_t* caller);

static void
consume_foreach(FILE*fd,
                const fcc_operator_t* foreach,
                const char* source,
                const fcc_operator_t* caller);

static void
consume_join(FILE*fd,
             const fcc_operator_t* join,
             const char* source,
             const fcc_operator_t* caller);

static void
consume_cross_join(FILE*fd,
                   const fcc_operator_t* cross_join,
                   const char* source,
                   const fcc_operator_t* caller);

static void
consume_leftfilter_join(FILE*fd,
                        const fcc_operator_t* left_filter_join,
                        const char* source,
                        const fcc_operator_t* caller);

static void
consume_gather(FILE*fd,
               const fcc_operator_t* gather,
               const char* source,
               const fcc_operator_t* caller);

static void
consume_cascading_gather(FILE*fd,
                         const fcc_operator_t* casc_gather,
                         const char* source,
                         const fcc_operator_t* caller);

static void
consume_fetch(FILE*fd,
              const fcc_operator_t* fetch,
              const char* source,
              const fcc_operator_t* caller);

static void
consume_tag_filter(FILE*fd,
                   const fcc_operator_t* tag_filter,
                   const char* source,
                   const fcc_operator_t* caller);

static void
consume_predicate_filter(FILE*fd,
                         const fcc_operator_t* predicate_filter,
                         const char* source,
                         const fcc_operator_t* caller);

static void
consume_component_filter(FILE*fd,
                         const fcc_operator_t* component_filter,
                         const char* source,
                         const fcc_operator_t* caller);

void 
consume(FILE*fd,
        const fcc_operator_t* fcc_operator,
        const char* source,
        const fcc_operator_t* caller)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      consume_scan(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_FOREACH:
      consume_foreach(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_JOIN:
      consume_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      consume_leftfilter_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      consume_cross_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_FETCH:
      consume_fetch(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_GATHER:
      consume_gather(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      consume_cascading_gather(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      consume_tag_filter(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      consume_predicate_filter(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      consume_component_filter(fd, fcc_operator, source, caller);
      break;
  };
}

void 
consume_foreach(FILE*fd, 
                const fcc_operator_t* foreach,
                const char* source,
                const fcc_operator_t* caller)
{

  bool all_globals = true;
  uint32_t ncols = foreach->m_num_columns;
  for(uint32_t i = 0; i < ncols; ++i)
  {
    if(foreach->m_columns[i].m_type != fcc_column_type_t::E_GLOBAL)
    {
      all_globals = false;
      break;
    }
  }


  int param_index = 0;
  for(uint32_t i = 0; i < ncols; ++i) 
  {
    const fcc_column_t* column = &foreach->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_ID)
    {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "<ForEach> operator cannot\
                                           be applied to reference column type: \"%s\"", 
                                           column->m_ref_name);
    }
    char tmp[FCC_MAX_QUALIFIED_TYPE_NAME];
    fcc_type_qualified_name(column->m_component_type,
                            tmp,
                            FCC_MAX_QUALIFIED_TYPE_NAME);

    switch(column->m_type)
    {
      case fcc_column_type_t::E_COMPONENT:
        fprintf(fd,
                "FDB_ALIGNED(%s*,data_%d,64) = (%s*)(%s->p_blocks[%d]);\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      case fcc_column_type_t::E_REFERENCE:
        fprintf(fd,
                "FDB_ALIGNED(%s**,data_%d,64) = (%s**)(%s->p_blocks[%d]);\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      case fcc_column_type_t::E_GLOBAL:
        fprintf(fd,
                "FDB_ALIGNED(%s*,data_%d, 64) = (%s*)(%s->p_blocks[%d]);\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      default:
        FDB_ASSERT(false && "Should not reach this point");
        break;
    }
    param_index++;
  }
  fprintf(fd, "\n");

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  //uint32_t size = foreach->p_systems.size();
  //for(uint32_t i = 0; i < size; ++i)
  {
    //const fcc_system_t* info = foreach->p_systems[i];
    const fcc_system_t* info = foreach->m_foreach.p_system;

    char system_name[FCC_MAX_TYPE_NAME];
    fcc_type_name(info->m_system_type, 
                  system_name, 
                  FCC_MAX_TYPE_NAME);


    char wrapper_name[FCC_MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 info->m_id,
                                 wrapper_name, 
                                 FCC_MAX_SYSTEM_WRAPPER_VARNAME);


    if(all_globals)
    {
      fdb_str_builder_append(&str_builder, "%s->run(&context,\n0", 
                         wrapper_name);
    }
    else
    {
      fdb_str_builder_append(&str_builder, "%s->run(&context,\n%s->m_start + i", 
                         wrapper_name, 
                         source);
    }

    for(size_t j = 0; j <  ncols; ++j) 
    {
      const fcc_column_t* column = &foreach->m_columns[j];
      switch(column->m_type)
      {
        case fcc_column_type_t::E_COMPONENT:
          // It is a component, thus we need to pass a pointer to data[i] 
          fdb_str_builder_append(&str_builder,",\n&data_%d[i]",j);
          break;
        case fcc_column_type_t::E_REFERENCE:
          // It is a reference, thus data[i] already contains a pointer
          fdb_str_builder_append(&str_builder,",\ndata_%d[i]",j);
          break;
        case fcc_column_type_t::E_GLOBAL:
          // It is a global, thus we need directly pass the data pointer
          fdb_str_builder_append(&str_builder,",\ndata_%d",j);
          break;
        default:
          FDB_ASSERT(false && "Should not reach this point");
          break;
      }
    }
    fdb_str_builder_append(&str_builder,");\n"); 
  }

  if(all_globals)
  {
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
  }
  else
  {
    fprintf(fd,
            "if(%s->m_enabled.m_num_set == FDB_TXTABLE_BLOCK_SIZE)\n{\n", 
            source);

    fprintf(fd,
            "for (size_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
    fprintf(fd,
            "}\n");
    fprintf(fd,
            "}\n");
    fprintf(fd,
            "else\n{\n");
    fprintf(fd,
            "for (size_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(fd,
            "if(fdb_bitmap_is_set(&%s->m_enabled, i))\n{\n", 
            source);
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
    fprintf(fd,
            "}\n");

    fprintf(fd,
            "}\n");

    fprintf(fd,
            "}\n");
  }
  fdb_str_builder_release(&str_builder);

  if(foreach->m_parent != FCC_INVALID_ID)
  {
    fcc_subplan_t* subplan = foreach->p_subplan;
    consume(fd,
            &subplan->m_nodes[foreach->m_parent],
            source,
            foreach);
  }

}

void 
consume_scan(FILE*fd,
             const fcc_operator_t* scan,
             const char* source,
             const fcc_operator_t* caller)
{
}

void
consume_join(FILE*fd, 
             const fcc_operator_t* join,
             const char* source,
             const fcc_operator_t* caller)
{

  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  if(caller->m_id == join->m_join.m_left) 
  {

    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 
    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");
    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster,%s);\n", 
            source); 
    fprintf(fd, 
            "fdb_btree_insert(&%s, cluster->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            hashtable); 
  }
  else 
  {

    fprintf(fd,
            "fdb_bcluster_t* build = (fdb_bcluster_t*)fdb_btree_get(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n",
            hashtable,
            source);

    fprintf(fd,
            "if(build != nullptr)\n{\n");
    char clustername[FCC_MAX_CLUSTER_VARNAME];
    generate_cluster_name(join,
                          clustername,
                          FCC_MAX_CLUSTER_VARNAME);

    fprintf(fd,
            "fdb_bcluster_t %s;\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_init(&%s, &bcluster_factory);\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_append_cluster(&%s, build);\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_append_cluster(&%s, %s);\n", 
            clustername, 
            source);
    fprintf(fd,
            "if(%s.m_enabled.m_num_set != 0)\n{\n", 
            clustername);

    fdb_str_builder_t str_builder;
    fdb_str_builder_init(&str_builder);
    fdb_str_builder_append(&str_builder, "(&%s)", clustername);
    fcc_subplan_t* subplan = join->p_subplan;
    consume(fd,
            &subplan->m_nodes[join->m_parent],
            str_builder.p_buffer,
            join);
    fdb_str_builder_release(&str_builder);

    fprintf(fd,"}\n");

    fprintf(fd,
            "fdb_bcluster_release(&%s);\n", 
            clustername);

    fprintf(fd,"}\n");
  }
}

void
consume_leftfilter_join(FILE*fd, 
                        const fcc_operator_t* left_filter_join,
                        const char* source,
                        const fcc_operator_t* caller)
{
  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(left_filter_join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  if(caller->m_id == left_filter_join->m_leftfilter_join.m_left) 
  {

    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");
    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster,%s);\n", 
            source); 

    fprintf(fd, 
            "fdb_btree_insert(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            hashtable, 
            source); 
  }
  else 
  {
    fprintf(fd,
            "fdb_bcluster_t* build = (fdb_bcluster_t*)fdb_btree_get(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n",
            hashtable,
            source);
    fprintf(fd,
            "if(build != nullptr)\n{\n");
    char clustername[FCC_MAX_CLUSTER_VARNAME];
    generate_cluster_name(left_filter_join,
                          clustername,
                          FCC_MAX_CLUSTER_VARNAME);

    fprintf(fd,
            "fdb_bcluster_t %s;\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_init(&%s, &bcluster_factory);\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_append_cluster(&%s, build);\n", 
            clustername);
    fprintf(fd,
            "fdb_bcluster_filter(&%s, %s);\n", 
            clustername, 
            source);
    fprintf(fd,
            "if(%s.m_enabled.m_num_set != 0)\n{\n", 
            clustername);

    fdb_str_builder_t str_builder;
    fdb_str_builder_init(&str_builder);
    fdb_str_builder_append(&str_builder, "(&%s)", clustername);
    fcc_subplan_t* subplan = left_filter_join->p_subplan;
    consume(fd,
            &subplan->m_nodes[left_filter_join->m_parent],
            str_builder.p_buffer,
            left_filter_join);
    fdb_str_builder_release(&str_builder);

    fprintf(fd,"}\n");

    fprintf(fd,
            "fdb_bcluster_release(&%s);\n", 
            clustername);
    fprintf(fd,"}\n");
  }
}

void
consume_cross_join(FILE*fd, 
                   const fcc_operator_t* join,
                   const char* source,
                   const fcc_operator_t* caller)
{

  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  if(caller->m_id == join->m_cross_join.m_left) 
  {
    fdb_str_builder_t str_builder;
    fdb_str_builder_init(&str_builder);
    fdb_str_builder_append(&str_builder,"left_%s", hashtable);

    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");
    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster,%s);\n", 
            source);

    fprintf(fd, 
            "fdb_btree_insert(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            str_builder.p_buffer, 
            source); 
    fdb_str_builder_release(&str_builder);
  }
  else 
  {
    fdb_str_builder_t str_builder;
    fdb_str_builder_init(&str_builder);
    fdb_str_builder_append(&str_builder,"right_%s", hashtable);

    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");
    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster,%s);\n", 
            source);

    fprintf(fd, 
            "fdb_btree_insert(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            str_builder.p_buffer, 
            source); 
    fdb_str_builder_release(&str_builder);
  }
}

void
consume_fetch(FILE*fd, 
              const fcc_operator_t* fetch,
              const char* source,
              const fcc_operator_t* caller)
{


}

void 
consume_tag_filter(FILE*fd, 
                   const fcc_operator_t* tag_filter,
                   const char* source,
                   const fcc_operator_t* caller)
{

  char fdb_bittable_name[FCC_MAX_TAG_TABLE_VARNAME];
  generate_bittable_name(tag_filter->m_tag_filter.m_tag,
                         fdb_bittable_name,
                         FCC_MAX_TAG_TABLE_VARNAME);
  fprintf(fd,"\n");

  if(!tag_filter->m_tag_filter.m_on_column)
  {
    fprintf(fd,
            "fdb_txbitmap_t* filter = fdb_txbittable_get_bitmap(%s, tx, txtctx, %s->m_start);\n", 
            fdb_bittable_name, 
            source);

    switch(tag_filter->m_tag_filter.m_op_type) 
    {
      case fcc_filter_op_type_t::E_HAS:
        {

          fprintf(fd,
                  "if(filter != nullptr){\n");
          fprintf(fd,
                  "fdb_bitmap_set_txbitmap_and(&%s->m_enabled, filter, tx, txtctx);\n",
                  source);
          fprintf(fd,
                  "}\n");
          fprintf(fd,
                  "else {\n");
          fprintf(fd,
                  "fdb_bitmap_nullify(&%s->m_enabled);\n",
                  source);
          fprintf(fd,
                  "}\n");
          break;
        }
      case fcc_filter_op_type_t::E_HAS_NOT:
        {

          fprintf(fd, 
                  "fdb_bitmap_t negate;\n");
          fprintf(fd, 
                  "fdb_bitmap_init(&negate, &bitmap_factory);\n");
          fprintf(fd,
                  "if(filter != nullptr){\n");
          fprintf(fd, 
                  "fdb_bitmap_set_txbitmap(&negate, filter, tx, txtctx);\n");
          fprintf(fd,
                  "}\n");
          fprintf(fd, 
                  "fdb_bitmap_negate(&negate);\n");
          fprintf(fd,
                  "fdb_bitmap_set_and(&%s->m_enabled, &negate);\n",
                  source);
          fprintf(fd, 
                  "fdb_bitmap_release(&negate);\n");
          break;
        }
    }
  }
  else
  {
    if(tag_filter->m_columns[0].m_type != fcc_column_type_t::E_ID)
    {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "Cannot apply filter tag \"%s\" on column\
                                           on a non-reference column type", 
                                           tag_filter->m_tag_filter.m_tag);
    }

    switch(tag_filter->m_tag_filter.m_op_type) 
    {
      case fcc_filter_op_type_t::E_HAS:
        {
          fprintf(fd,
                  "filter_txbittable_exists(%s,tx, txtctx, %s,0);\n",
                  fdb_bittable_name,
                  source);
          break;
        }
      case fcc_filter_op_type_t::E_HAS_NOT:
        {
          fprintf(fd,
                  "filter_txbittable_not_exists(%s,tx, txtctx, %s,0);\n",
                  fdb_bittable_name,
                  source);
          break;
        }
    }
  }

  fprintf(fd,
          "if(%s->m_enabled.m_num_set != 0)\n{\n",
          source); 
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[tag_filter->m_parent],
          source,
          tag_filter);
  fprintf(fd,
          "}\n");
}

void
consume_component_filter(FILE*fd, 
                         const fcc_operator_t* component_filter,
                         const char* source,
                         const fcc_operator_t* caller)
{
  FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                       "Component filter not yet implemented");
  // if ...
  fcc_subplan_t* subplan = component_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[component_filter->m_parent],
          "cluster",
          component_filter);
}

void
consume_predicate_filter(FILE*fd, 
                         const fcc_operator_t* predicate_filter,
                         const char* source,
                         const fcc_operator_t* caller)
{
  // if ...
  fprintf(fd,"\n");
  int param_index = 0;
  uint32_t ncols = predicate_filter->m_num_columns;
  for(uint32_t i = 0; i < ncols; ++i)  
  {
    const fcc_column_t* column = &predicate_filter->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_ID)
    {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "<PredicateFilter> operator cannot be applied to \
                                           reference column type: \"%s\"", 
                                           column->m_ref_name);
    }

    char tmp[FCC_MAX_QUALIFIED_TYPE_NAME];
    fcc_type_qualified_name(column->m_component_type,
                            tmp,
                            FCC_MAX_QUALIFIED_TYPE_NAME);

    switch(column->m_type)
    {
      case fcc_column_type_t::E_COMPONENT:
        fprintf(fd,
                "FDB_ALIGNED(%s*,data_%d,64) = (%s*)(%s->p_blocks[%d]);\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      case fcc_column_type_t::E_REFERENCE:
        fprintf(fd,
                "FDB_ALIGNED(%s**, data_%d, 64) = (%s**)(%s->p_blocks[%d]));\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      case fcc_column_type_t::E_GLOBAL:
        fprintf(fd,
                "FDB_ALIGNED(%s*, data_%d, 64) = (%s*)(%s->p_blocks[%d]);\n", 
                tmp, 
                param_index, 
                tmp, 
                source, 
                param_index);
        break;
      default:
        FDB_ASSERT(false && "Should not reach this point");
        break;
    }
    param_index++;
  }

  if(fcc_decl_is_function(predicate_filter->m_predicate_filter.m_func_decl) && 
     fcc_decl_is_member(predicate_filter->m_predicate_filter.m_func_decl))
  {
    uint32_t code_length = 4096;
    char* code = new char[code_length];
    uint32_t length = 0;
    while((length = fcc_decl_code(predicate_filter->m_predicate_filter.m_func_decl, code, code_length)) >= code_length)
    {
      delete [] code;
      code_length *= 2;
      code = new char[code_length];
    }

    fprintf(fd, "%s", code);
    delete [] code;
  }

  char func_name[2048];
  fcc_decl_function_name(predicate_filter->m_predicate_filter.m_func_decl, 
                         func_name, 
                         2048);

  fprintf(fd,
          "for(uint32_t i = 0; i < FDB_TXTABLE_BLOCK_SIZE && (%s->m_enabled.m_num_set != 0); ++i) \n{\n",
          source);
  fprintf(fd,
          "fdb_bitmap_set_bit(&%s->m_enabled, i, fdb_bitmap_is_set(&%s->m_enabled, i) && %s(",
          source,
          source,
          func_name);

  const fcc_column_t* column = &predicate_filter->m_columns[0];

  switch(column->m_type)
  {
    case fcc_column_type_t::E_COMPONENT:
      // It is a component, thus we need to pass a pointer to data[i] 
      fprintf(fd,"&data_0[i]");
      break;
    case fcc_column_type_t::E_REFERENCE:
      // It is a reference, thus data[i] already contains a pointer
      fprintf(fd,"data_0[i]");
      break;
    case fcc_column_type_t::E_GLOBAL:
      // It is a global, thus we need directly pass the data pointer
      fprintf(fd,"data_0");
      break;
    default:
      FDB_ASSERT(false && "Should not reach this point");
      break;
  }

  for(size_t i = 1; i < ncols; ++i)
  {
    const fcc_column_t* column = &predicate_filter->m_columns[i];
    switch(column->m_type)
    {
      case fcc_column_type_t::E_COMPONENT:
        // It is a component, thus we need to pass a pointer to data[i] 
        fprintf(fd,",&data_%zu[i]", i);
        break;
      case fcc_column_type_t::E_REFERENCE:
        // It is a reference, thus data[i] already contains a pointer
        fprintf(fd,",data_%zu[i]", i);
        break;
      case fcc_column_type_t::E_GLOBAL:
        // It is a global, thus we need directly pass the data pointer
        fprintf(fd,",data_%zu", i);
        break;
      default:
        FDB_ASSERT(false && "Should not reach this point");
        break;
    }
  }
  fprintf(fd, "));\n");
  fprintf(fd, "}\n");
  fprintf(fd,
          "if(%s->m_enabled.m_num_set != 0)\n{\n",
          source); 
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[predicate_filter->m_parent],
          source,
          predicate_filter);
  fprintf(fd, "}\n");
}

void
consume_gather(FILE*fd, 
               const fcc_operator_t* gather,
               const char* source,
               const fcc_operator_t* caller)
{
  fcc_subplan_t* subplan = gather->p_subplan;

  if(caller->m_id == gather->m_gather.m_ref_table) 
  {
    // FILLING UP TEMPORAL TABLES
    fprintf(fd,
            "fdb_tmptable_t* temp_tables[] = {");
    fcc_operator_t* child = &subplan->m_nodes[gather->m_gather.m_child];
    fcc_column_t* child_columns = child->m_columns;
    uint32_t ncols = child->m_num_columns;
    for(uint32_t i = 0; i < ncols; ++i)
    {
      fcc_column_t* column = &child_columns[i];
      char ctype[FCC_MAX_TYPE_NAME];
      fcc_type_name(column->m_component_type,
                    ctype,
                    FCC_MAX_TYPE_NAME); 

      char tablename[FCC_MAX_TABLE_VARNAME];
      generate_temp_table_name(ctype, tablename, 
                               FCC_MAX_TABLE_VARNAME,
                               gather);


      fprintf(fd,"&%s, ",
              tablename);
    }
    fprintf(fd,"nullptr};\n");

    fprintf(fd,
            "gather(tx, txtctx, %s,hash_tables, chunk_size, stride, temp_tables, %d);\n",
            source,
            ncols);
  }
  else
  {
    char hashtable[FCC_MAX_HASHTABLE_VARNAME];
    generate_hashtable_name(gather,
                            hashtable,
                            FCC_MAX_HASHTABLE_VARNAME);


    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");

    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster, %s);\n", 
            source); 

    fprintf(fd, 
            "fdb_btree_insert(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            hashtable, 
            source); 
  }
}

void
consume_cascading_gather(FILE*fd, 
                         const fcc_operator_t* casc_gather,
                         const char* source,
                         const fcc_operator_t* caller)
{

  fcc_subplan_t* subplan = casc_gather->p_subplan;
  if(caller->m_id == casc_gather->m_cascading_gather.m_ref_table) 
  {
    fprintf(fd,
            "find_roots_and_blacklist(%s, &next_frontier_%u, &partial_blacklist_%u);\n", 
            source,
            casc_gather->m_id,
            casc_gather->m_id);

    // FILLING UP TEMPORAL TABLES
    fprintf(fd,
            "fdb_tmptable_t* temp_tables[] = {");

    fcc_operator_t* child = &subplan->m_nodes[casc_gather->m_gather.m_child];
    fcc_column_t* child_columns = child->m_columns;
    uint32_t ncols = child->m_num_columns;
    for(uint32_t i = 0; i < ncols; ++i)
    {
      fcc_column_t* column = &child_columns[i];
      char ctype[FCC_MAX_TYPE_NAME];
      fcc_type_name(column->m_component_type,
                    ctype,
                    FCC_MAX_TYPE_NAME); 

      char tablename[FCC_MAX_TABLE_VARNAME];
      generate_temp_table_name(ctype, tablename, 
                               FCC_MAX_TABLE_VARNAME,
                               casc_gather);


      fprintf(fd,"&%s, ",
              tablename);
    }
    fprintf(fd,"nullptr};\n");

    fprintf(fd,
            "gather(tx, txtctx, %s, hash_tables, chunk_size, stride, temp_tables, %d);\n",
            source,
            ncols);

    char hashtable[FCC_MAX_HASHTABLE_VARNAME];
    generate_hashtable_name(casc_gather,
                            hashtable,
                            FCC_MAX_HASHTABLE_VARNAME);


    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");

    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster, %s);\n", 
            source); 

    fprintf(fd, 
            "fdb_btree_insert(&ref_%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            hashtable, 
            source); 
  }
  else
  {
    char hashtable[FCC_MAX_HASHTABLE_VARNAME];
    generate_hashtable_name(casc_gather,
                            hashtable,
                            FCC_MAX_HASHTABLE_VARNAME);


    fprintf(fd, 
            "fdb_bcluster_t* cluster = (fdb_bcluster_t*)mem_alloc(&task_allocator.m_super, 64, sizeof(fdb_bcluster_t), %s->m_start / FDB_TXTABLE_BLOCK_SIZE);\n", 
            source); 

    fprintf(fd, 
            "fdb_bcluster_init(cluster, &bcluster_factory);\n");

    fprintf(fd, 
            "fdb_bcluster_append_cluster(cluster, %s);\n", 
            source); 

    fprintf(fd, 
            "fdb_btree_insert(&%s, %s->m_start / FDB_TXTABLE_BLOCK_SIZE, cluster);\n", 
            hashtable, 
            source); 


  }
}

