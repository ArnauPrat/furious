


#include "../../common/str_builder.h"
#include "../frontend/operator.h"
#include "../driver.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "consumer.h"
#include "producer.h"

#include <stdlib.h>

namespace furious 
{

void
produce(FILE* fd, 
        const fcc_operator_t* fcc_operator,
        bool parallel_stream);

static void
produce_scan(FILE* fd,
        const fcc_operator_t* scan,
        bool parallel_stream);

static void
produce_foreach(FILE* fd,
                const fcc_operator_t* foreach,
                bool parallel_stream);

static void
produce_join(FILE* fd,
             const fcc_operator_t* join,
             bool parallel_stream);

static void
produce_cross_join(FILE* fd,
        const fcc_operator_t* cross_join,
        bool parallel_stream);

static void
produce_leftfilter_join(FILE* fd,
                        const fcc_operator_t* left_filter_join,
                        bool parallel_stream);

static void
produce_gather(FILE* fd,
               const fcc_operator_t* gather,
               bool parallel_stream);

static void
produce_cascading_gather(FILE* fd,
                         const fcc_operator_t* casc_gather,
                         bool parallel_stream);

static void
produce_fetch(FILE* fd,
              const fcc_operator_t* fetch,
              bool parallel_stream);

static void
produce_tag_filter(FILE* fd,
        const fcc_operator_t* tag_filter,
        bool parallel_stream);

static void
produce_predicate_filter(FILE* fd,
        const fcc_operator_t* predicate_filter,
        bool parallel_stream);

static void
produce_component_filter(FILE* fd,
        const fcc_operator_t* component_filter,
        bool parallel_stream);

void 
produce(FILE* fd,
        const fcc_operator_t* fcc_operator,
        bool parallel_stream)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      produce_scan(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_FOREACH:
      produce_foreach(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_JOIN:
      produce_join(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      produce_leftfilter_join(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      produce_cross_join(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_FETCH:
      produce_fetch(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_GATHER:
      produce_gather(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      produce_cascading_gather(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      produce_tag_filter(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      produce_predicate_filter(fd, fcc_operator, parallel_stream);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      produce_component_filter(fd, fcc_operator, parallel_stream);
      break;
  };
}


void 
produce_foreach(FILE* fd,
                const fcc_operator_t* foreach,
                bool parallel_stream)
{

  bool all_globals = true;
  for(uint32_t i = 0; i < foreach->m_columns.size(); ++i)
  {
    if(foreach->m_columns[i].m_type != fcc_column_type_t::E_GLOBAL)
    {
      all_globals = false;
      break;
    }
  }

  if(all_globals)
  {
    fprintf(fd, 
            "if(context.m_thread_id == 0)\n{\n");
  }

  fcc_subplan_t* subplan = foreach->p_subplan;

  produce(fd,
          &subplan->m_nodes[foreach->m_foreach.m_child],
          true);

  if(all_globals)
  {
    fprintf(fd, 
            "}\n");
  }
}

void 
produce_scan(FILE* fd,
        const fcc_operator_t* scan,
        bool parallel_stream)
{
  static uint32_t id = 0;
  char tablename[MAX_TABLE_VARNAME];
  char blockname[MAX_BLOCK_VARNAME];
  char itername[MAX_ITER_VARNAME];

  const fcc_column_t* column = &scan->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char q_ctype[MAX_QUALIFIED_TYPE_NAME];
    length = fcc_type_qualified_name(column->m_component_type,
                                                     q_ctype,
                                                     MAX_QUALIFIED_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(length, MAX_QUALIFIED_TYPE_NAME);

    length = generate_table_name(ctype,
                        tablename,
                        MAX_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    length = generate_table_iter_name(tablename, 
                                      itername,
                                      MAX_ITER_VARNAME,
                                      scan);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    id++;
    length = generate_block_name(ctype,
                                 blockname,
                                 MAX_BLOCK_VARNAME,
                                 scan); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_BLOCK_VARNAME);

  }
  else
  {
    uint32_t length = generate_ref_table_name(column[0].m_ref_name,
                                              tablename,
                                              MAX_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    length = generate_table_iter_name(tablename, 
                                      itername, 
                                      MAX_ITER_VARNAME, 
                                      scan);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    id++;
    length = generate_block_name(column[0].m_ref_name,
                                 blockname, 
                                 MAX_BLOCK_VARNAME, 
                                 scan); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_BLOCK_VARNAME);


  }

  if(parallel_stream)
  {
    fprintf(fd,
            "auto %s = %s.iterator(chunk_size, offset, stride);\n",
            itername,
            tablename);
  }
  else
  {
    fprintf(fd,
            "auto %s = %s.iterator(1, 0, 1);\n",
            itername,
            tablename);
  }

  fprintf(fd,
          "while(%s.has_next())\n\
          {\n\
          BlockCluster %s(%s.next().get_raw());\n",
          itername,
          blockname,
          itername);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, 
                     "(&%s)", 
                     blockname);

  fcc_subplan_t* subplan = scan->p_subplan;
  consume(fd,
          &subplan->m_nodes[scan->m_parent],
          str_builder.p_buffer,
          scan);

  str_builder_release(&str_builder);

  fprintf(fd,
          "}\n\n");
}

void
produce_join(FILE* fd,
             const fcc_operator_t* join, 
             bool parallel_stream)
{
  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(fd,"BTree<BlockCluster> %s;\n", hashtable);
  fcc_subplan_t* subplan = join->p_subplan;
  produce(fd,&subplan->m_nodes[join->m_join.m_left], parallel_stream);
  produce(fd,&subplan->m_nodes[join->m_join.m_right], parallel_stream);
}

void
produce_leftfilter_join(FILE* fd,
                        const fcc_operator_t* left_filter_join,
                        bool parallel_stream)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(left_filter_join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(fd,"BTree<BlockCluster> %s;\n", hashtable);
  fcc_subplan_t* subplan = left_filter_join->p_subplan;
  produce(fd,&subplan->m_nodes[left_filter_join->m_leftfilter_join.m_left], parallel_stream);
  produce(fd,&subplan->m_nodes[left_filter_join->m_leftfilter_join.m_right], parallel_stream);
}

void
produce_cross_join(FILE* fd,
                   const fcc_operator_t* cross_join,
                   bool parallel_stream)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  uint32_t length = generate_hashtable_name(cross_join,
                                            hashtable,
                                            MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  str_builder_t str_builder_left;
  str_builder_init(&str_builder_left);
  str_builder_append(&str_builder_left, "left_%s", hashtable);
  fprintf(fd,"BTree<BlockCluster> %s;\n", str_builder_left.p_buffer);

  fcc_subplan_t* subplan = cross_join->p_subplan;
  produce(fd,&subplan->m_nodes[cross_join->m_cross_join.m_left], parallel_stream);

  str_builder_t str_builder_right;
  str_builder_init(&str_builder_right);
  str_builder_append(&str_builder_right, "right_%s", hashtable);
  fprintf(fd,"BTree<BlockCluster> %s;\n", str_builder_right.p_buffer);
  produce(fd,&subplan->m_nodes[cross_join->m_cross_join.m_right], parallel_stream);

  fprintf(fd, "auto left_iter_hashtable_%d = %s.iterator();\n", 
          cross_join->m_id, 
          str_builder_left.p_buffer);
  fprintf(fd, "while(left_iter_hashtable_%d.has_next())\n{\n", cross_join->m_id);

  char cluster[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(cross_join, 
                                 cluster, 
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  str_builder_t str_builder_cluster_left;
  str_builder_init(&str_builder_cluster_left);
  str_builder_append(&str_builder_cluster_left,"left_%s", cluster);
  fprintf(fd,
          "BlockCluster* %s = left_iter_hashtable_%d.next().p_value;\n", 
          str_builder_cluster_left.p_buffer,
          cross_join->m_id);

  fprintf(fd, "auto right_iter_hashtable_%d = %s.iterator();\n", 
          cross_join->m_id, 
          str_builder_right.p_buffer);
  fprintf(fd, "while(right_iter_hashtable_%d.has_next())\n{\n", cross_join->m_id);

  str_builder_t str_builder_cluster_right;
  str_builder_init(&str_builder_cluster_right);
  str_builder_append(&str_builder_cluster_right,"right_%s", cluster);
  fprintf(fd,
          "BlockCluster* %s = right_iter_hashtable_%d.next().p_value;\n", 
          str_builder_cluster_right.p_buffer,
          cross_join->m_id);


  char joined_cluster[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(cross_join,
                                 joined_cluster,
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(fd,
          "BlockCluster %s(*%s);\n", 
          joined_cluster,
          str_builder_cluster_left.p_buffer);

  fprintf(fd,
          "%s.append(%s);\n", 
          joined_cluster,
          str_builder_cluster_right.p_buffer);

  str_builder_t str_builder_joined_cluster;
  str_builder_init(&str_builder_joined_cluster);
  str_builder_append(&str_builder_joined_cluster,"(&%s)", joined_cluster);
  consume(fd,
          &subplan->m_nodes[cross_join->m_parent],
          str_builder_joined_cluster.p_buffer,
          cross_join);

  fprintf(fd, 
          "}\n"); 
  fprintf(fd, 
          "}\n"); 

  str_builder_release(&str_builder_joined_cluster);
  str_builder_release(&str_builder_cluster_left);
  str_builder_release(&str_builder_cluster_right);
  str_builder_release(&str_builder_left);
  str_builder_release(&str_builder_right);
}

void
produce_fetch(FILE* fd,
              const fcc_operator_t* fetch, 
              bool parallel_stream)
{

  fprintf(fd, 
          "{\n"); 

  char globaltype[MAX_TYPE_NAME];
  const uint32_t type_length = fcc_type_name(fetch->m_columns[0].m_component_type, 
                                             globaltype, 
                                             MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(type_length, MAX_TYPE_NAME);

  char globalname[MAX_TYPE_NAME];
  const uint32_t global_length = generate_global_name(globaltype,
                                                      globalname,
                                                      MAX_TYPE_NAME,
                                                      fetch);
  FURIOUS_CHECK_STR_LENGTH(global_length, MAX_TYPE_NAME);

  char clustername[MAX_CLUSTER_VARNAME];
  const uint32_t cluster_length = generate_cluster_name(fetch,
                                                        clustername,
                                                        MAX_CLUSTER_VARNAME);
  FURIOUS_CHECK_STR_LENGTH(cluster_length, MAX_CLUSTER_VARNAME);

  fprintf(fd, 
          "%s* %s = database->find_global_no_lock<%s>();\n", 
          globaltype,
          globalname,
          globaltype);

  fprintf(fd, 
          "if(%s != nullptr )\n{\n", 
          globalname);

  fprintf(fd, 
          "BlockCluster %s;\n", 
          clustername); 

  fprintf(fd, 
          "%s.append_global(%s);\n", 
          clustername,
          globalname); 

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "(&%s)", clustername);
  fcc_subplan_t* subplan = fetch->p_subplan;
  consume(fd,
          &subplan->m_nodes[fetch->m_parent],
          str_builder.p_buffer,
          fetch);

  str_builder_release(&str_builder);


  fprintf(fd, 
          "}\n"); 

  fprintf(fd, 
          "}\n"); 

}

void 
produce_tag_filter(FILE* fd,
                   const fcc_operator_t* tag_filter,
                   bool parallel_stream)
{
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  produce(fd, &subplan->m_nodes[tag_filter->m_tag_filter.m_child], parallel_stream);
}

void
produce_component_filter(FILE* fd,
                         const fcc_operator_t* component_filter, 
                         bool parallel_stream)
{
  fcc_subplan_t* subplan = component_filter->p_subplan;
  produce(fd, &subplan->m_nodes[component_filter->m_component_filter.m_child], 
          parallel_stream);
}

void
produce_predicate_filter(FILE* fd,
                         const fcc_operator_t* predicate_filter, 
                         bool parallel_stream)
{
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  produce(fd, &subplan->m_nodes[predicate_filter->m_predicate_filter.m_child], 
          parallel_stream);
}

void
produce_gather(FILE* fd,
               const fcc_operator_t* gather,
               bool parallel_stream)
{
  char refname[MAX_REF_TABLE_VARNAME];
  fcc_subplan_t* subplan = gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[gather->m_gather.m_ref_table];
  generate_ref_groups_name(ref_table->m_columns[0].m_ref_name, 
                           refname,
                           MAX_REF_TABLE_VARNAME,
                           gather);

  fprintf(fd,"BTree<DynArray<entity_id_t> > %s;\n",  refname);
  produce(fd, ref_table, parallel_stream);

  // This is a temporal buffer needed for generating temporal table names from
  // threading parameters to make them unique
  fprintf(fd,"char tmp_buffer_%d[256];\n", gather->m_id);

  // Generating temporal tables
  fcc_operator_t* child = &subplan->m_nodes[gather->m_gather.m_child];
  DynArray<fcc_column_t>& child_columns = child->m_columns;
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    if(child_columns[i].m_type == fcc_column_type_t::E_REFERENCE)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                         "<Gather> operator target component cannot be a\
                         reference column type: \"%s\"", 
                         column->m_ref_name);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    if(child_columns[i].m_access_mode != fcc_access_mode_t::E_READ)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                         "<Gather> operator target component must have \
                         access mode READ: \"%s\"",
                         ctype);

      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }
    char temptablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      temptablename, 
                                      MAX_TABLE_VARNAME, 
                                      gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    fprintf(fd,"snprintf(tmp_buffer_%d, 256-1, \"%s_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
            gather->m_id, 
            temptablename);

    fprintf(fd,"TableView<%s> %s = database->create_temp_table<%s>(tmp_buffer_%d);\n", 
            ctype,
            temptablename,
            ctype,
            gather->m_id);

    fprintf(fd,"%s.clear();\n", 
            temptablename);
  }

  produce(fd, child, parallel_stream);

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      tablename, 
                                      MAX_TABLE_VARNAME, 
                                      gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    char itername[MAX_ITER_VARNAME];
    length = generate_table_iter_name(tablename, 
                                      itername,
                                      MAX_ITER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);


    if(parallel_stream)
    {
      fprintf(fd, "auto %s = %s.iterator(chunk_size, offset, stride);\n", 
              itername, 
              tablename);
    }
    else
    {
      fprintf(fd, "auto %s = %s.iterator(1, 0, 1);\n", 
              itername, 
              tablename);
    }
  }

  fcc_column_t* column = &child_columns[0];

  char ctype[MAX_TYPE_NAME];
  uint32_t length = fcc_type_name(column->m_component_type,
                                  ctype,
                                  MAX_TYPE_NAME); 

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

  char tablename[MAX_TABLE_VARNAME];
  length = generate_temp_table_name(ctype, 
                                    tablename,
                                    MAX_TABLE_VARNAME,
                                    gather);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

  char itername[MAX_ITER_VARNAME];
  length = generate_table_iter_name(tablename,
                                    itername,
                                    MAX_ITER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

  fprintf(fd, "while(%s.has_next())\n{\n", itername);

  char clustername[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(gather,
                                 clustername,
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(fd, "BlockCluster %s;\n", clustername);

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      tablename,
                                      MAX_TABLE_VARNAME,
                                      gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    char itername[MAX_ITER_VARNAME];
    length = generate_table_iter_name(tablename,
                                      itername,
                                      MAX_ITER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    fprintf(fd, "%s.append(%s.next().get_raw());\n", 
            clustername, 
            itername);
  }

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "(&%s)", clustername);
  consume(fd,
          &subplan->m_nodes[gather->m_parent],
          str_builder.p_buffer,
          gather);
  str_builder_release(&str_builder);

  fprintf(fd,"}\n\n");

}

void
produce_cascading_gather(FILE* fd,
                         const fcc_operator_t* casc_gather, 
                         bool parallel_stream)
{
  // GENERATING REFERENCE GROUPS
  fprintf(fd, "FURIOUS_PERMA_ASSERT(stride == 1 || (stride > 1 && barrier != nullptr));\n");


  char groups[MAX_REF_TABLE_VARNAME];
  fcc_subplan_t* subplan = casc_gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[casc_gather->m_gather.m_ref_table];
  uint32_t length = generate_ref_groups_name(ref_table->m_columns[0].m_ref_name, 
                                             groups, 
                                             MAX_REF_TABLE_VARNAME,
                                             casc_gather);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_REF_TABLE_VARNAME);

  fprintf(fd,"BTree<DynArray<entity_id_t> > %s;\n", groups);
  produce(fd, ref_table, false);

  // GENERATING HASHTABLE WITH CHILD BLOCKS 
  char hashtable[MAX_HASHTABLE_VARNAME];
  length = generate_hashtable_name(casc_gather,
                                   hashtable,
                                   MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(fd,"BTree<BlockCluster> %s;\n", hashtable);
  fcc_operator_t* child = &subplan->m_nodes[casc_gather->m_gather.m_child];
  produce(fd, child, false);

  // This is a temporal buffer needed for generating temporal table names from
  // threading parameters to make them unique
  fprintf(fd,"char tmp_buffer_%d[256];\n", casc_gather->m_id);

  // CREATE TEMPORAL TABLES
  DynArray<fcc_column_t>& child_columns = child->m_columns;
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    if(child_columns[i].m_type == fcc_column_type_t::E_REFERENCE)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                         "<Gather> operator target component cannot\
                         be a reference column type: \"%s\"", 
                         column->m_ref_name);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    if(child_columns[i].m_access_mode != fcc_access_mode_t::E_READ)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                         "<Gather> operator target component \
                         must have access mode READ: \"%s\"",
                         ctype);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }
    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      tablename,
                                      MAX_TABLE_VARNAME,
                                      casc_gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    fprintf(fd,"snprintf(tmp_buffer_%d, 256-1, \"%s_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
            casc_gather->m_id,
            tablename);

    fprintf(fd,"TableView<%s> %s = database->create_temp_table<%s>(tmp_buffer_%d);\n", 
            ctype,
            tablename,
            ctype,
            casc_gather->m_id);
  }

  // DECLARE BITTABLES FOR CASCADING
  fprintf(fd,"BitTable bittable1_%u;\n", casc_gather->m_id);
  fprintf(fd,"BitTable bittable2_%u;\n", casc_gather->m_id);
  fprintf(fd,
          "BitTable* current_frontier_%u = &bittable1_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "BitTable* next_frontier_%u = &bittable2_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "find_roots(&%s, current_frontier_%u);\n", 
          groups,
          casc_gather->m_id);

  fprintf(fd,
          "int32_t last_barrier = 0;");

  fprintf(fd,
          "while(current_frontier_%u->size() > 0)\n{\n", 
          casc_gather->m_id);


  // CLEARING TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      tablename, 
                                      MAX_TABLE_VARNAME,
                                      casc_gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    fprintf(fd,"%s.clear();\n", 
            tablename);
  }

  // FILLING UP TEMPORAL TABLES

  str_builder_t str_builder_iter;
  str_builder_init(&str_builder_iter);
  str_builder_append(&str_builder_iter, "iter_hashtable_%d",casc_gather->m_id);
  fprintf(fd, "auto %s = %s.iterator();\n", 
          str_builder_iter.p_buffer, 
          hashtable);

  fprintf(fd, "while(%s.has_next())\n{\n", str_builder_iter.p_buffer);
  fprintf(fd,
          "gather(&%s,%s.next().p_value,current_frontier_%u, next_frontier_%u",
          groups,
          str_builder_iter.p_buffer,
          casc_gather->m_id,
          casc_gather->m_id);
  str_builder_release(&str_builder_iter);


  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, tablename, 
                                      MAX_TABLE_VARNAME,
                                      casc_gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    fprintf(fd,",&%s",
            tablename);
  }
  fprintf(fd,");\n");


  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,"}\n");


  // ITERATE OVER TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype, 
                                      tablename, 
                                      MAX_TABLE_VARNAME, 
                                      casc_gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    char itername[MAX_ITER_VARNAME];
    length = generate_table_iter_name(tablename, 
                                      itername, 
                                      MAX_ITER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    if(parallel_stream)
    {
      fprintf(fd, "auto %s = %s.iterator(chunk_size, offset, stride);\n", 
              itername, 
              tablename);
    }
    else
    {
      fprintf(fd, "auto %s = %s.iterator(1, 0, 1);\n", 
              itername, 
              tablename);
    }
  }

  fcc_column_t* column = &child_columns[0];
  char ctype[MAX_TYPE_NAME];
  length = fcc_type_name(column->m_component_type,
                         ctype,
                         MAX_TYPE_NAME); 

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

  char tablename[MAX_TABLE_VARNAME];
  length = generate_temp_table_name(ctype,
                                    tablename,
                                    MAX_TABLE_VARNAME,
                                    casc_gather);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

  char itername[MAX_ITER_VARNAME];
  length = generate_table_iter_name(tablename,
                                    itername,
                                    MAX_ITER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

  fprintf(fd, "while(%s.has_next())\n{\n", itername);

  char clustername[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(casc_gather,
                                 clustername,
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(fd, "BlockCluster %s;\n", clustername);

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[MAX_TYPE_NAME];
    uint32_t length = fcc_type_name(column->m_component_type,
                                    ctype,
                                    MAX_TYPE_NAME); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    char tablename[MAX_TABLE_VARNAME];
    length = generate_temp_table_name(ctype,
                                      tablename,
                                      MAX_TABLE_VARNAME,
                                      casc_gather);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    char itername[MAX_ITER_VARNAME];
    length = generate_table_iter_name(tablename,
                                      itername,
                                      MAX_ITER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    fprintf(fd, "%s.append(%s.next().get_raw());\n", 
            clustername, 
            itername);
  }

  str_builder_t str_builder_cluster;
  str_builder_init(&str_builder_cluster);
  str_builder_append(&str_builder_cluster, "(&%s)", clustername);
  consume(fd,
          &subplan->m_nodes[casc_gather->m_parent],
          str_builder_cluster.p_buffer,
          casc_gather);
  str_builder_release(&str_builder_cluster);

  fprintf(fd,"}\n");

  //SWAP FRONTIERS
  fprintf(fd,
          "BitTable* temp_frontier_%u = current_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "current_frontier_%u = next_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "next_frontier_%u = temp_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "next_frontier_%u->clear();\n", 
          casc_gather->m_id);

  fprintf(fd,
          "}\n\n");

}

} /* furious */ 
