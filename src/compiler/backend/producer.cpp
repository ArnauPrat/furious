


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
  char tablename[FCC_MAX_TABLE_VARNAME];
  char blockname[FCC_MAX_BLOCK_VARNAME];
  char itername[FCC_MAX_ITER_VARNAME];

  const fcc_column_t* column = &scan->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                                    ctype,
                                    FCC_MAX_TYPE_NAME);


    char q_ctype[FCC_MAX_QUALIFIED_TYPE_NAME];
    fcc_type_qualified_name(column->m_component_type,
                                                     q_ctype,
                                                     FCC_MAX_QUALIFIED_TYPE_NAME);

    generate_table_name(ctype,
                        tablename,
                        FCC_MAX_TABLE_VARNAME);


    generate_table_iter_name(tablename, 
                                      itername,
                                      FCC_MAX_ITER_VARNAME,
                                      scan);


    id++;
    generate_block_name(ctype,
                                 blockname,
                                 FCC_MAX_BLOCK_VARNAME,
                                 scan); 


  }
  else
  {
    generate_ref_table_name(column[0].m_ref_name,
                            tablename,
                            FCC_MAX_TABLE_VARNAME);


    generate_table_iter_name(tablename, 
                             itername, 
                             FCC_MAX_ITER_VARNAME, 
                             scan);

    id++;
    generate_block_name(column[0].m_ref_name,
                        blockname, 
                        FCC_MAX_BLOCK_VARNAME, 
                        scan); 

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
          block_cluster_t %s = block_cluster_create(&task_allocator);\n",
          itername,
          blockname);
  fprintf(fd, 
          "block_cluster_append(&%s,%s.next().get_raw());\n",
          blockname,
          itername
         );

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, 
                     "(&%s)", 
                     blockname);

  fcc_subplan_t* subplan = scan->p_subplan;
  consume(fd,
          &subplan->m_nodes[scan->m_parent],
          str_builder.p_buffer,
          scan);

  str_builder_destroy(&str_builder);

  fprintf(fd, 
          "block_cluster_destroy(&%s, &task_allocator);\n",
          blockname);

  fprintf(fd,
          "}\n\n");
}

void
produce_join(FILE* fd,
             const fcc_operator_t* join, 
             bool parallel_stream)
{
  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);


  fprintf(fd,
          "btree_t %s = btree_create(&task_allocator);\n", 
          hashtable);

  fcc_subplan_t* subplan = join->p_subplan;
  produce(fd,&subplan->m_nodes[join->m_join.m_left], parallel_stream);


  produce(fd,&subplan->m_nodes[join->m_join.m_right], parallel_stream);



  fprintf(fd,
          "btree_iter_t iter_%s = btree_iter_create(&%s);\n",
          hashtable,
          hashtable);

  fprintf(fd, 
          "while(btree_iter_has_next(&iter_%s))\n{\n",
          hashtable);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&iter_%s).p_value;\n", 
          hashtable);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,
          "btree_destroy(&%s);\n", 
          hashtable);
}

void
produce_leftfilter_join(FILE* fd,
                        const fcc_operator_t* left_filter_join,
                        bool parallel_stream)
{

  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(left_filter_join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  fprintf(fd,
          "btree_t %s = btree_create(&task_allocator);\n", 
          hashtable);

  fcc_subplan_t* subplan = left_filter_join->p_subplan;
  produce(fd,&subplan->m_nodes[left_filter_join->m_leftfilter_join.m_left], parallel_stream);
  produce(fd,&subplan->m_nodes[left_filter_join->m_leftfilter_join.m_right], parallel_stream);

  fprintf(fd,
          "btree_iter_t it_%s = btree_iter_create(&%s);\n", 
          hashtable,
          hashtable);

  fprintf(fd,
          "while(btree_iter_has_next(&it_%s))\n{\n", hashtable);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_%s).p_value;\n", 
          hashtable);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");
  fprintf(fd,"btree_iter_destroy(&it_%s);\n", hashtable);
  fprintf(fd,"btree_destroy(&%s);\n", hashtable);
}

void
produce_cross_join(FILE* fd,
                   const fcc_operator_t* cross_join,
                   bool parallel_stream)
{

  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(cross_join,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);


  str_builder_t str_builder_left = str_builder_create();
  str_builder_append(&str_builder_left, "left_%s", hashtable);
  fprintf(fd,
          "btree_t %s = btree_create(&task_allocator);\n", 
          str_builder_left.p_buffer);

  fcc_subplan_t* subplan = cross_join->p_subplan;
  produce(fd,&subplan->m_nodes[cross_join->m_cross_join.m_left], parallel_stream);

  str_builder_t str_builder_right = str_builder_create();
  str_builder_append(&str_builder_right, "right_%s", hashtable);
  fprintf(fd,
          "btree_t %s = btree_create(&task_allocator);\n", 
          str_builder_right.p_buffer);
  produce(fd,&subplan->m_nodes[cross_join->m_cross_join.m_right], parallel_stream);

  fprintf(fd, "btree_iter_t left_iter_hashtable_%d = btree_iter_create(&%s);\n", 
          cross_join->m_id, 
          str_builder_left.p_buffer);

  fprintf(fd, "while(btree_iter_has_next(&left_iter_hashtable_%d))\n{\n", cross_join->m_id);

  char cluster[FCC_MAX_CLUSTER_VARNAME];
  generate_cluster_name(cross_join, 
                        cluster, 
                        FCC_MAX_CLUSTER_VARNAME);

  str_builder_t str_builder_cluster_left = str_builder_create();
  str_builder_append(&str_builder_cluster_left,"left_%s", cluster);
  fprintf(fd,
          "block_cluster_t* %s = (block_cluster_t*)btree_iter_next(&left_iter_hashtable_%d).p_value;\n", 
          str_builder_cluster_left.p_buffer,
          cross_join->m_id);

  fprintf(fd, "btree_iter_t right_iter_hashtable_%d = btree_iter_create(&%s);\n", 
          cross_join->m_id,
          str_builder_right.p_buffer);

  fprintf(fd, "while(btree_iter_has_next(&right_iter_hashtable_%d))\n{\n", cross_join->m_id);

  str_builder_t str_builder_cluster_right = str_builder_create();
  str_builder_append(&str_builder_cluster_right,"right_%s", cluster);
  fprintf(fd,
          "block_cluster_t* %s = (block_cluster_t*)btree_iter_next(&right_iter_hashtable_%d).p_value;\n", 
          str_builder_cluster_right.p_buffer,
          cross_join->m_id);


  char joined_cluster[FCC_MAX_CLUSTER_VARNAME];
  generate_cluster_name(cross_join,
                        joined_cluster,
                        FCC_MAX_CLUSTER_VARNAME);


  fprintf(fd,
          "block_cluster_t %s = block_cluster_create(&task_allocator);\n", 
          joined_cluster);

  fprintf(fd,
          "block_cluster_append(&%s,%s);\n", 
          joined_cluster,
          str_builder_cluster_left.p_buffer);

  fprintf(fd,
          "block_cluster_append(&%s, %s);\n", 
          joined_cluster,
          str_builder_cluster_right.p_buffer);

  str_builder_t str_builder_joined_cluster = str_builder_create();
  str_builder_append(&str_builder_joined_cluster,"(&%s)", joined_cluster);
  consume(fd,
          &subplan->m_nodes[cross_join->m_parent],
          str_builder_joined_cluster.p_buffer,
          cross_join);

  fprintf(fd,
          "block_cluster_destroy(&%s, &task_allocator);\n", 
          joined_cluster);

  fprintf(fd, 
          "}\n"); 
  fprintf(fd,"btree_iter_destroy(&right_iter_hashtable_%d);\n", 
          cross_join->m_id);
  fprintf(fd, 
          "}\n"); 

  fprintf(fd,
          "btree_iter_t it_%s = btree_iter_create(&%s);\n", 
          str_builder_left.p_buffer,
          str_builder_left.p_buffer);
  fprintf(fd,
          "while(btree_iter_has_next(&it_%s))\n{\n", str_builder_left.p_buffer);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_%s).p_value;\n", 
          str_builder_left.p_buffer);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,"btree_iter_destroy(&it_%s);\n", 
          str_builder_left.p_buffer);
  fprintf(fd,"btree_iter_destroy(&left_iter_hashtable_%d);\n", 
          cross_join->m_id);
  fprintf(fd,"btree_destroy(&%s);\n", str_builder_left.p_buffer);

  fprintf(fd,
          "btree_iter_t it_%s = btree_iter_create(&%s);\n", 
          str_builder_right.p_buffer,
          str_builder_right.p_buffer);

  fprintf(fd,
          "while(btree_iter_has_next(&it_%s))\n{\n", str_builder_right.p_buffer);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_%s).p_value;\n", 
          str_builder_right.p_buffer);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");
  fprintf(fd,"btree_iter_destroy(&it_%s);\n", 
          str_builder_right.p_buffer);
  fprintf(fd,"btree_destroy(&%s);\n", str_builder_right.p_buffer);

  str_builder_destroy(&str_builder_joined_cluster);
  str_builder_destroy(&str_builder_cluster_left);
  str_builder_destroy(&str_builder_cluster_right);
  str_builder_destroy(&str_builder_left);
  str_builder_destroy(&str_builder_right);
}

void
produce_fetch(FILE* fd,
              const fcc_operator_t* fetch, 
              bool parallel_stream)
{

  fprintf(fd, 
          "{\n"); 

  char globaltype[FCC_MAX_TYPE_NAME];
  fcc_type_name(fetch->m_columns[0].m_component_type, 
                globaltype, 
                FCC_MAX_TYPE_NAME);

  char globalname[FCC_MAX_TYPE_NAME];
  generate_global_name(globaltype,
                       globalname,
                       FCC_MAX_TYPE_NAME,
                       fetch);

  char clustername[FCC_MAX_CLUSTER_VARNAME];
  generate_cluster_name(fetch,
                        clustername,
                        FCC_MAX_CLUSTER_VARNAME);

  fprintf(fd, 
          "%s* %s = FURIOUS_FIND_GLOBAL_NO_LOCK(database, %s);\n", 
          globaltype,
          globalname,
          globaltype);

  fprintf(fd, 
          "if(%s != nullptr )\n{\n", 
          globalname);

  fprintf(fd, 
          "block_cluster_t %s = block_cluster_create(&task_allocator);\n", 
          clustername); 

  fprintf(fd, 
          "block_cluster_append_global(&%s, %s);\n", 
          clustername,
          globalname); 

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "(&%s)", clustername);
  fcc_subplan_t* subplan = fetch->p_subplan;
  consume(fd,
          &subplan->m_nodes[fetch->m_parent],
          str_builder.p_buffer,
          fetch);

  str_builder_destroy(&str_builder);

  fprintf(fd, 
          "block_cluster_destroy(&%s, &task_allocator);\n", 
          clustername); 

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
  fcc_subplan_t* subplan = gather->p_subplan;

  fprintf(fd,
          "int32_t last_barrier = 0;\n");

  // This is a temporal buffer needed for generating temporal table names from
  // threading parameters to make them unique
  fprintf(fd,
          "char tmp_buffer_%d_%d[256];\n", 
          gather->p_subplan->m_id,
          gather->m_id);


  // FILLING UP HASHTABLE WITH CHILD BLOCKS 
  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(gather,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  // DECLARE HASH TABLE
  fprintf(fd,"btree_t %s = btree_create(&task_allocator);\n", hashtable);

  // REGISTER HASHTABLE TO REGISTRY FOR MULTITHREADED EXECUTION
  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
          gather->p_subplan->m_id,
          gather->m_id,
          hashtable,
          gather->p_subplan->m_id,
          gather->m_id);
  fprintf(fd,
          "ht_registry_insert(&ht_registry, tmp_buffer_%d_%d, &%s);\n",
          gather->p_subplan->m_id, 
          gather->m_id,
          hashtable);

  fcc_operator_t* child = &subplan->m_nodes[gather->m_gather.m_child];
  produce(fd, child, parallel_stream);

  // CREATING TEMPORAL TABLES 
  DynArray<fcc_column_t>& child_columns = child->m_columns;
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    if(child_columns[i].m_type == fcc_column_type_t::E_ID)
    {

      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                         "<Gather> operator target component cannot be a\
                         reference column type: \"%s\"", 
                         column->m_ref_name);
    }

    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME);

    if(child_columns[i].m_access_mode != fcc_access_mode_t::E_READ)
    {

      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "<Gather> operator target component must have \
                                           access mode READ: \"%s\"",
                                           ctype);
    }
    char temptablename[FCC_MAX_TABLE_VARNAME];
    generate_temp_table_name(ctype, 
                             temptablename, 
                             FCC_MAX_TABLE_VARNAME, 
                             gather);

    fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
            gather->p_subplan->m_id,
            gather->m_id, 
            temptablename,
            gather->p_subplan->m_id,
            gather->m_id);

    fprintf(fd,"TableView<%s*> %s = FURIOUS_CREATE_TEMP_TABLE(database, %s*, tmp_buffer_%d_%d);\n", 
            ctype,
            temptablename,
            ctype,
            gather->p_subplan->m_id,
            gather->m_id);

    fprintf(fd,"%s.clear();\n", 
            temptablename);
  }

  // SYNCHRONIZING THREADS TO MAKE SURE HASHTABLES ARE AVAILABLE
  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd, 
          "DynArray<FURIOUS_RESTRICT(btree_t*)> hash_tables;\n");
  fprintf(fd, 
          "for(uint32_t i = 0; i < stride; ++i)\n");
  fprintf(fd, 
          "{\n");
  // REGISTER HASHTABLE TO REGISTRY FOR MULTITHREADED EXECUTION
  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, i, stride);\n", 
          gather->p_subplan->m_id,
          gather->m_id,
          hashtable,
          gather->p_subplan->m_id,
          gather->m_id);
  fprintf(fd,
          "btree_t* ht = (btree_t*)ht_registry_get(&ht_registry, tmp_buffer_%d_%d);\n",
          gather->p_subplan->m_id, 
          gather->m_id);
  fprintf(fd, 
          "hash_tables.append(ht);\n");
  fprintf(fd, 
          "}\n");

  // PRODUCE REFERENCE TABLE
  fcc_operator_t* ref_table = &subplan->m_nodes[gather->m_gather.m_ref_table];
  produce(fd, ref_table, parallel_stream);

  // DEFININT ITERATORS TO TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME);


    char tablename[FCC_MAX_TABLE_VARNAME];
    generate_temp_table_name(ctype, 
                             tablename, 
                             FCC_MAX_TABLE_VARNAME, 
                             gather);

    char itername[FCC_MAX_ITER_VARNAME];
    generate_table_iter_name(tablename, 
                             itername,
                             FCC_MAX_ITER_VARNAME);

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

  char ctype[FCC_MAX_TYPE_NAME];
  fcc_type_name(column->m_component_type,
                ctype,
                FCC_MAX_TYPE_NAME); 

  char tablename[FCC_MAX_TABLE_VARNAME];
  generate_temp_table_name(ctype, 
                           tablename,
                           FCC_MAX_TABLE_VARNAME,
                           gather);


  char itername[FCC_MAX_ITER_VARNAME];
  generate_table_iter_name(tablename,
                           itername,
                           FCC_MAX_ITER_VARNAME);

  // ITERATE OVER TEMPORAL TABLES AND PRODUCE CLUSTERS FOR CALLER OPERATORS
  fprintf(fd, "while(%s.has_next())\n{\n", itername);

  char clustername[FCC_MAX_CLUSTER_VARNAME];
  generate_cluster_name(gather,
                        clustername,
                        FCC_MAX_CLUSTER_VARNAME);

  fprintf(fd, "block_cluster_t %s = block_cluster_create(&task_allocator);\n", clustername);

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME); 

    char tablename[FCC_MAX_TABLE_VARNAME];
    generate_temp_table_name(ctype, 
                             tablename,
                             FCC_MAX_TABLE_VARNAME,
                             gather);

    char itername[FCC_MAX_ITER_VARNAME];
    generate_table_iter_name(tablename,
                             itername,
                             FCC_MAX_ITER_VARNAME);

    fprintf(fd, "block_cluster_append(&%s, %s.next().get_raw());\n", 
            clustername, 
            itername);
  }

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "(&%s)", clustername);
  consume(fd,
          &subplan->m_nodes[gather->m_parent],
          str_builder.p_buffer,
          gather);
  str_builder_destroy(&str_builder);

  fprintf(fd, "block_cluster_destroy(&%s, &task_allocator);\n", clustername);

  fprintf(fd,"}\n\n");

  // SYNCHRONIZING THREADS TO MAKE SURE HASHTABLES ARE AVAILABLE
  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,
          "btree_iter_t it_%s = btree_iter_create(&%s);\n", 
          hashtable, 
          hashtable);


  fprintf(fd,
          "while(btree_iter_has_next(&it_%s))\n{\n", hashtable);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_%s).p_value;\n", 
          hashtable);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");
  fprintf(fd,
          "btree_iter_destroy(&it_%s);\n", 
          hashtable);
  fprintf(fd,"btree_destroy(&%s);\n", hashtable);

}

void
produce_cascading_gather(FILE* fd,
                         const fcc_operator_t* casc_gather, 
                         bool parallel_stream)
{
  fprintf(fd, "FURIOUS_PERMA_ASSERT(stride == 1 || (stride > 1 && barrier != nullptr));\n");
  fprintf(fd,
          "int32_t last_barrier = 0;");

  fcc_subplan_t* subplan = casc_gather->p_subplan;

  // This is a temporal buffer needed for generating temporal table names from
  // threading parameters to make them unique
  fprintf(fd,
          "char tmp_buffer_%d_%d[256];\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);

  // GENERATING HASHTABLE WITH CHILD BLOCKS 
  char hashtable[FCC_MAX_HASHTABLE_VARNAME];
  generate_hashtable_name(casc_gather,
                          hashtable,
                          FCC_MAX_HASHTABLE_VARNAME);

  // DECLARE HASH TABLE
  fprintf(fd,"btree_t %s = btree_create(&task_allocator);\n", hashtable);

  // REGISTER HASHTABLE TO REGISTRY FOR MULTITHREADED EXECUTION
  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          hashtable,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);
  fprintf(fd,
          "ht_registry_insert(&ht_registry, tmp_buffer_%d_%d, &%s);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id,
          hashtable);


  // PRODUCE CHILD BLOCKS
  fcc_operator_t* child = &subplan->m_nodes[casc_gather->m_gather.m_child];
  produce(fd, child, true);


  // DECLARE BITTABLES FOR CASCADING
  fprintf(fd,
          "BitTable* current_frontier_%u = new BitTable(&task_allocator);\n", 
          casc_gather->m_id);

  fprintf(fd,
          "BitTable* next_frontier_%u = new BitTable(&task_allocator);\n", 
          casc_gather->m_id);

  fprintf(fd,
          "BitTable* partial_blacklist_%u= new BitTable(&task_allocator);\n", 
          casc_gather->m_id);

  DynArray<fcc_column_t>& child_columns = child->m_columns;
  char** temp_table_names = new char*[child_columns.size()];
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    temp_table_names[i] = new char[FCC_MAX_TABLE_VARNAME];
  }

  // CREATE TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];

    if(child_columns[i].m_type == fcc_column_type_t::E_ID)
    {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "<Gather> operator target component cannot\
                                           be a reference column type: \"%s\"", 
                                           column->m_ref_name);
    }

    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME);

    if(child_columns[i].m_access_mode != fcc_access_mode_t::E_READ)
    {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                           "<Gather> operator target component \
                                           must have access mode READ: \"%s\"",
                                           ctype);
    }

    generate_temp_table_name(ctype, 
                             temp_table_names[i],
                             FCC_MAX_TABLE_VARNAME,
                             casc_gather);

  }

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME);

    fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
            casc_gather->p_subplan->m_id,
            casc_gather->m_id,
            temp_table_names[i],
            casc_gather->p_subplan->m_id,
            casc_gather->m_id);

    fprintf(fd,"TableView<%s*> %s = FURIOUS_CREATE_TEMP_TABLE(database, %s*, tmp_buffer_%d_%d);\n", 
            ctype,
            temp_table_names[i],
            ctype,
            casc_gather->p_subplan->m_id,
            casc_gather->m_id);

    fprintf(fd,"%s.clear();\n", 
            temp_table_names[i]);
  }

  // SYNCHRONIZING THREADS TO MAKE SURE HASHTABLES ARE AVAILABLE
  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd, 
          "DynArray<FURIOUS_RESTRICT(btree_t*)> hash_tables;\n");
  fprintf(fd, 
          "for(uint32_t i = 0; i < stride; ++i)\n");
  fprintf(fd, 
          "{\n");
  // REGISTER HASHTABLE TO REGISTRY FOR MULTITHREADED EXECUTION
  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"%s_%d_%d_%%d_%%d_%%d\", chunk_size, i, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          hashtable,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);
  fprintf(fd,
          "btree_t* ht = (btree_t*)ht_registry_get(&ht_registry, tmp_buffer_%d_%d);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id);
  fprintf(fd, 
          "hash_tables.append(ht);\n");
  fprintf(fd, 
          "}\n");

  // DECLARE HASH TABLE
  fprintf(fd,"btree_t ref_%s = btree_create(&task_allocator);\n", hashtable);

  // PRODUCING REFERENCE TABLE
  fcc_operator_t* ref_table = &subplan->m_nodes[casc_gather->m_gather.m_ref_table];
  produce(fd, ref_table, parallel_stream);

  // REGISTERING PARTIAL BLACKLIST AND NEXT FRONTIERS
  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"blacklist_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);
  fprintf(fd,
          "ht_registry_insert(&ht_registry, tmp_buffer_%d_%d, partial_blacklist_%u);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id,
          casc_gather->m_id);


  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"next_frontier_%d_%d_%%d_%%d_%%d\", chunk_size, offset, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);
  fprintf(fd,
          "ht_registry_insert(&ht_registry, tmp_buffer_%d_%d, next_frontier_%u);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id,
          casc_gather->m_id);

  // SYNCHRONIZING THREADS TO MAKE SURE PARTIAL BLACKLISTS ARE AVAILABLE
  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  // BROADCASTING NEXT FRONTIERS TO CURRENT FRONTIER
  
  fprintf(fd, 
          "DynArray<FURIOUS_RESTRICT(BitTable*)> blacklists_%u;\n",
          casc_gather->m_id);

  fprintf(fd, 
          "DynArray<FURIOUS_RESTRICT(BitTable*)> next_frontiers_%u;\n",
          casc_gather->m_id);
  fprintf(fd,
          "for(uint32_t i = 0; i < stride; ++i)\n{\n");

  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"blacklist_%d_%d_%%d_%%d_%%d\", chunk_size, i, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);

  
  fprintf(fd,
          "BitTable* other_blacklist = (BitTable*)ht_registry_get(&ht_registry, tmp_buffer_%d_%d);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id);

  fprintf(fd,
          "blacklists_%u.append(other_blacklist);\n", 
          casc_gather->m_id);

  fprintf(fd,"snprintf(tmp_buffer_%d_%d, 256-1, \"next_frontier_%d_%d_%%d_%%d_%%d\", chunk_size, i, stride);\n", 
          casc_gather->p_subplan->m_id,
          casc_gather->m_id,
          casc_gather->p_subplan->m_id,
          casc_gather->m_id);

  
  fprintf(fd,
          "BitTable* other_frontier = (BitTable*)ht_registry_get(&ht_registry, tmp_buffer_%d_%d);\n",
          casc_gather->p_subplan->m_id, 
          casc_gather->m_id);

  fprintf(fd,
          "next_frontiers_%u.append(other_frontier);\n", 
          casc_gather->m_id);

  fprintf(fd,
          "}\n");

  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,
          "frontiers_union(&next_frontiers_%u, current_frontier_%u);\n", 
          casc_gather->m_id,
          casc_gather->m_id);
  fprintf(fd,
          "filter_blacklists(&blacklists_%u, current_frontier_%u);\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");



  fprintf(fd,
          "while(current_frontier_%u->size() > 0)\n{\n", 
          casc_gather->m_id);

  fprintf(fd,
          "next_frontier_%u->clear();\n",
          casc_gather->m_id);
  


  fcc_column_t* column = &child_columns[0];
  char ctype[FCC_MAX_TYPE_NAME];
  fcc_type_name(column->m_component_type,
                ctype,
                FCC_MAX_TYPE_NAME); 


  char tablename[FCC_MAX_TABLE_VARNAME];
  generate_temp_table_name(ctype,
                           tablename,
                           FCC_MAX_TABLE_VARNAME,
                           casc_gather);


  char itername[FCC_MAX_ITER_VARNAME];
  generate_table_iter_name(tablename,
                           itername,
                           FCC_MAX_ITER_VARNAME);

  fprintf(fd, "btree_iter_t iter_ref_hashtable_%d = btree_iter_create(&ref_%s);\n", 
          casc_gather->m_id, 
          hashtable);

  fprintf(fd, "while(btree_iter_has_next(&iter_ref_hashtable_%d))\n{\n", casc_gather->m_id);

  char clustername[FCC_MAX_CLUSTER_VARNAME];
  generate_cluster_name(casc_gather,
                        clustername,
                        FCC_MAX_CLUSTER_VARNAME);

  fprintf(fd, "block_cluster_t %s = block_cluster_create(&task_allocator);\n", clustername);
  fprintf(fd, 
          "build_block_cluster_from_refs((block_cluster_t*)btree_iter_next(&iter_ref_hashtable_%d).p_value,"
          "current_frontier_%u,"
          "next_frontier_%u,"
          "&%s",
          casc_gather->m_id, 
          casc_gather->m_id,
          casc_gather->m_id,
          clustername);

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fprintf(fd, 
            ",&%s", 
            temp_table_names[i]);
  }
  fprintf(fd, 
         ");\n");

  str_builder_t str_builder_cluster = str_builder_create();
  str_builder_append(&str_builder_cluster, "(&%s)", clustername);
  consume(fd,
          &subplan->m_nodes[casc_gather->m_parent],
          str_builder_cluster.p_buffer,
          casc_gather);
  str_builder_destroy(&str_builder_cluster);

  fprintf(fd, "block_cluster_destroy(&%s, &task_allocator);\n", 
          clustername);
  fprintf(fd,"}\n");

  fprintf(fd, "btree_iter_destroy(&iter_ref_hashtable_%d);\n", 
          casc_gather->m_id);

  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");

  fprintf(fd,
          "current_frontier_%u->clear();\n",
          casc_gather->m_id);
  fprintf(fd,
          "frontiers_union(&next_frontiers_%u, current_frontier_%u);\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(fd,
          "if(stride > 1)\n{\n");
  fprintf(fd,
          "last_barrier += stride;\n");
  fprintf(fd,
          "barrier->wait(last_barrier);\n");
  fprintf(fd,
          "}\n");


  fprintf(fd,
          "}\n\n");

  fprintf(fd,
          "btree_iter_t it_%s = btree_iter_create(&%s);\n", 
          hashtable, 
          hashtable);

  fprintf(fd,
          "while(btree_iter_has_next(&it_%s))\n{\n", hashtable);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_%s).p_value;\n", 
          hashtable);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");
  fprintf(fd,"btree_iter_destroy(&it_%s);\n", hashtable);
  fprintf(fd,"btree_destroy(&%s);\n", hashtable);

  fprintf(fd,
          "btree_iter_t it_ref_%s = btree_iter_create(&ref_%s);\n", 
          hashtable, 
          hashtable);

  fprintf(fd,
          "while(btree_iter_has_next(&it_ref_%s))\n{\n", hashtable);
  fprintf(fd,
          "block_cluster_t* next = (block_cluster_t*)btree_iter_next(&it_ref_%s).p_value;\n", 
          hashtable);
  fprintf(fd,
          "block_cluster_destroy(next, &task_allocator);\n");
  fprintf(fd,
          "mem_free(&task_allocator,next);\n");
  fprintf(fd,
          "}\n");
  fprintf(fd,"btree_iter_destroy(&it_ref_%s);\n", hashtable);
  fprintf(fd,"btree_destroy(&ref_%s);\n", hashtable);

  fprintf(fd,"delete current_frontier_%u;\n", 
          casc_gather->m_id);

  fprintf(fd,"delete next_frontier_%u;\n", 
          casc_gather->m_id);

  fprintf(fd,"delete partial_blacklist_%u;\n", 
          casc_gather->m_id);

  // CLEARING TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    fcc_column_t* column = &child_columns[i];
    char ctype[FCC_MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  FCC_MAX_TYPE_NAME); 


    char tablename[FCC_MAX_TABLE_VARNAME];
    generate_temp_table_name(ctype, 
                             tablename, 
                             FCC_MAX_TABLE_VARNAME,
                             casc_gather);

    fprintf(fd,"%s.clear();\n", 
            tablename);
  }

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    delete [] temp_table_names[i];
  }
  delete [] temp_table_names;

}

} /* furious */ 
