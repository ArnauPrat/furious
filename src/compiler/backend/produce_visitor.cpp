

#include "../../common/str_builder.h"
#include "../drivers/clang/clang_tools.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "consume_visitor.h"
#include "../frontend/operator.h"
#include "produce_visitor.h"

#include <stdlib.h>
#include <string>
#include <algorithm>

namespace furious 
{

ProduceVisitor::ProduceVisitor(CodeGenContext* context) :
p_context{context}
{
}

void 
ProduceVisitor::visit(const Foreach* foreach)
{
  produce(p_context->p_fd,
          foreach->p_child.get());
}

void 
ProduceVisitor::visit(const Scan* scan)
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

    std::string base_name = ctype;

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
    std::string base_name = column[0].m_ref_name;
    uint32_t length = generate_ref_table_name(base_name.c_str(),
                                              tablename,
                                              MAX_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    length = generate_table_iter_name(tablename, 
                                      itername, 
                                      MAX_ITER_VARNAME, 
                                      scan);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_ITER_VARNAME);

    id++;
    length = generate_block_name(base_name.c_str(),
                                 blockname, 
                                 MAX_BLOCK_VARNAME, 
                                 scan); 

    FURIOUS_CHECK_STR_LENGTH(length, MAX_BLOCK_VARNAME);
  }

  fprintf(p_context->p_fd,
          "auto %s = %s.iterator(chunk_size, offset, stride);\n\
           while(%s.has_next())\n\
           {\n\
           BlockCluster %s(%s.next().get_raw());\n",
           itername,
           tablename,
           itername,
           blockname,
           itername);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, 
                        "(&%s)", 
                        blockname);

  consume(p_context->p_fd,
          scan->p_parent,
          str_builder.p_buffer,
          scan);

  str_builder_release(&str_builder);

  fprintf(p_context->p_fd,
          "}\n\n");
}

void
ProduceVisitor::visit(const Join* join)
{
  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable);
  produce(p_context->p_fd,join->p_left.get());
  produce(p_context->p_fd,join->p_right.get());
}

void
ProduceVisitor::visit(const LeftFilterJoin* left_filter_join)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(left_filter_join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable);
  produce(p_context->p_fd,left_filter_join->p_left.get());
  produce(p_context->p_fd,left_filter_join->p_right.get());
}

void
ProduceVisitor::visit(const CrossJoin* cross_join)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  uint32_t length = generate_hashtable_name(cross_join,
                                            hashtable,
                                            MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  str_builder_t str_builder_left;
  str_builder_init(&str_builder_left);
  str_builder_append(&str_builder_left, "left_%s", hashtable);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", str_builder_left.p_buffer);

  produce(p_context->p_fd,cross_join->p_left.get());

  str_builder_t str_builder_right;
  str_builder_init(&str_builder_right);
  str_builder_append(&str_builder_right, "right_%s", hashtable);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", str_builder_right.p_buffer);
  produce(p_context->p_fd,cross_join->p_right.get());

  std::string iter_varname_left = "left_iter_hashtable_"+std::to_string(cross_join->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          iter_varname_left.c_str(), 
          str_builder_left.p_buffer);
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname_left.c_str());

  char cluster[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(cross_join, 
                                 cluster, 
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  str_builder_t str_builder_cluster_left;
  str_builder_init(&str_builder_cluster_left);
  str_builder_append(&str_builder_cluster_left,"left_%s", cluster);
  fprintf(p_context->p_fd,
          "BlockCluster* %s = %s.next().p_value;\n", 
          str_builder_cluster_left.p_buffer,
          iter_varname_left.c_str());

  std::string iter_varname_right = "right_iter_hashtable_"+std::to_string(cross_join->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          iter_varname_right.c_str(), 
          str_builder_right.p_buffer);
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname_right.c_str());

  str_builder_t str_builder_cluster_right;
  str_builder_init(&str_builder_cluster_right);
  str_builder_append(&str_builder_cluster_right,"right_%s", cluster);
  fprintf(p_context->p_fd,
          "BlockCluster* %s = %s.next().p_value;\n", 
          str_builder_cluster_right.p_buffer,
          iter_varname_right.c_str());


  char joined_cluster[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(cross_join,
                        joined_cluster,
                        MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(p_context->p_fd,
          "BlockCluster %s(*%s);\n", 
          joined_cluster,
          str_builder_cluster_left.p_buffer);

  fprintf(p_context->p_fd,
          "%s.append(%s);\n", 
          joined_cluster,
          str_builder_cluster_right.p_buffer);

  str_builder_t str_builder_joined_cluster;
  str_builder_init(&str_builder_joined_cluster);
  str_builder_append(&str_builder_joined_cluster,"(&%s)", joined_cluster);
  consume(p_context->p_fd,
          cross_join->p_parent,
          str_builder_joined_cluster.p_buffer,
          cross_join);

  fprintf(p_context->p_fd, 
          "}\n"); 
  fprintf(p_context->p_fd, 
          "}\n"); 

  str_builder_release(&str_builder_joined_cluster);
  str_builder_release(&str_builder_cluster_left);
  str_builder_release(&str_builder_cluster_right);
  str_builder_release(&str_builder_left);
  str_builder_release(&str_builder_right);
}

void
ProduceVisitor::visit(const Fetch* fetch)
{

  fprintf(p_context->p_fd, 
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

  fprintf(p_context->p_fd, 
          "%s* %s = database->find_global_no_lock<%s>();\n", 
          globaltype,
          globalname,
          globaltype);

  fprintf(p_context->p_fd, 
          "if(%s != nullptr )\n{\n", 
          globalname);

  fprintf(p_context->p_fd, 
          "BlockCluster %s;\n", 
          clustername); 

  fprintf(p_context->p_fd, 
          "%s.append_global(%s);\n", 
          clustername,
          globalname); 

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "(&%s)", clustername);
  consume(p_context->p_fd,
          fetch->p_parent,
          str_builder.p_buffer,
          fetch);

  str_builder_release(&str_builder);


  fprintf(p_context->p_fd, 
          "}\n"); 

  fprintf(p_context->p_fd, 
          "}\n"); 

}

void 
ProduceVisitor::visit(const TagFilter* tag_filter)
{
  produce(p_context->p_fd,tag_filter->p_child.get());
}

void
ProduceVisitor::visit(const ComponentFilter* component_filter)
{
  produce(p_context->p_fd,component_filter->p_child.get());
}

void
ProduceVisitor::visit(const PredicateFilter* predicate_filter)
{
  produce(p_context->p_fd,predicate_filter->p_child.get());
}

void
ProduceVisitor::visit(const Gather* gather)
{
  char refname[MAX_REF_TABLE_VARNAME];
  generate_ref_groups_name(gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                           refname,
                           MAX_REF_TABLE_VARNAME,
                           gather);

  fprintf(p_context->p_fd,"BTree<DynArray<entity_id_t> > %s;\n",  refname);
  produce(p_context->p_fd, gather->p_ref_table.get());

  // Generating temporal tables
  DynArray<fcc_column_t>& child_columns = gather->p_child.get()->m_columns;
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

    fprintf(p_context->p_fd,"TableView<%s> %s = database->create_temp_table_no_lock<%s>(\"%s\");\n", 
            ctype,
            temptablename,
            ctype,
            temptablename);

    fprintf(p_context->p_fd,"%s.clear();\n", 
            temptablename);
  }

  produce(p_context->p_fd, gather->p_child.get());

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


    fprintf(p_context->p_fd, "auto %s = %s.iterator(chunk_size, offset, stride);\n", 
            itername, 
            tablename);
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

  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", itername);

  char clustername[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(gather,
                        clustername,
                        MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(p_context->p_fd, "BlockCluster %s;\n", clustername);

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

    fprintf(p_context->p_fd, "%s.append(%s.next().get_raw());\n", 
            clustername, 
            itername);
  }

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "(&%s)", clustername);
  consume(p_context->p_fd,
          gather->p_parent,
          str_builder.p_buffer,
          gather);
  str_builder_release(&str_builder);

  fprintf(p_context->p_fd,"}\n\n");

}

void
ProduceVisitor::visit(const CascadingGather* casc_gather)
{
  // GENERATING REFERENCE GROUPS

  char groups[MAX_REF_TABLE_VARNAME];
  uint32_t length = generate_ref_groups_name(casc_gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                           groups, 
                           MAX_REF_TABLE_VARNAME,
                           casc_gather);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_REF_TABLE_VARNAME);

  fprintf(p_context->p_fd,"BTree<DynArray<entity_id_t> > %s;\n", groups);
  produce(p_context->p_fd, casc_gather->p_ref_table.get());

  // GENERATING HASHTABLE WITH CHILD BLOCKS 
  char hashtable[MAX_HASHTABLE_VARNAME];
  length = generate_hashtable_name(casc_gather,
                                   hashtable,
                                   MAX_HASHTABLE_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_HASHTABLE_VARNAME);

  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable);
  produce(p_context->p_fd, casc_gather->p_child.get());

  // CREATE TEMPORAL TABLES
  DynArray<fcc_column_t>& child_columns = casc_gather->p_child.get()->m_columns;
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

    fprintf(p_context->p_fd,"TableView<%s> %s = database->create_temp_table_no_lock<%s>(\"%s\");\n", 
            ctype,
            tablename,
            ctype,
            tablename);
  }

  // DECLARE BITTABLES FOR CASCADING
  fprintf(p_context->p_fd,"BitTable bittable1_%u;\n", casc_gather->m_id);
  fprintf(p_context->p_fd,"BitTable bittable2_%u;\n", casc_gather->m_id);
  fprintf(p_context->p_fd,
          "BitTable* current_frontier_%u = &bittable1_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "BitTable* next_frontier_%u = &bittable2_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "find_roots(&%s, current_frontier_%u);\n", 
          groups,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
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

    fprintf(p_context->p_fd,"%s.clear();\n", 
            tablename);
  }

  // FILLING UP TEMPORAL TABLES

  str_builder_t str_builder_iter;
  str_builder_init(&str_builder_iter);
  str_builder_append(&str_builder_iter, "iter_hashtable_%d",casc_gather->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          str_builder_iter.p_buffer, 
          hashtable);

  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", str_builder_iter.p_buffer);
  fprintf(p_context->p_fd,
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

    fprintf(p_context->p_fd,",&%s",
            tablename);
  }
  fprintf(p_context->p_fd,");\n");
  fprintf(p_context->p_fd,"}\n");


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

    fprintf(p_context->p_fd, "auto %s = %s.iterator(chunk_size, offset, stride);\n", 
            itername, 
            tablename);
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

  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", itername);

  char clustername[MAX_CLUSTER_VARNAME];
  length = generate_cluster_name(casc_gather,
                                 clustername,
                                 MAX_CLUSTER_VARNAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_CLUSTER_VARNAME);

  fprintf(p_context->p_fd, "BlockCluster %s;\n", clustername);

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

    fprintf(p_context->p_fd, "%s.append(%s.next().get_raw());\n", 
            clustername, 
            itername);
  }

  str_builder_t str_builder_cluster;
  str_builder_init(&str_builder_cluster);
  str_builder_append(&str_builder_cluster, "(&%s)", clustername);
  consume(p_context->p_fd,
          casc_gather->p_parent,
          str_builder_cluster.p_buffer,
          casc_gather);
  str_builder_release(&str_builder_cluster);

  fprintf(p_context->p_fd,"}\n");

  //SWAP FRONTIERS
  fprintf(p_context->p_fd,
          "BitTable* temp_frontier_%u = current_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "current_frontier_%u = next_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "next_frontier_%u = temp_frontier_%u;\n", 
          casc_gather->m_id,
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "next_frontier_%u->clear();\n", 
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "}\n\n");

}

} /* furious */ 
