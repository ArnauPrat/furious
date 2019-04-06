

#include "codegen.h"
#include "produce_visitor.h"
#include "consume_visitor.h"
#include "../clang_tools.h"
#include "codegen_tools.h"
#include "../../common/string_builder.h"

#include <stdlib.h>
#include <clang/AST/PrettyPrinter.h>
#include <string>
#include <algorithm>

using namespace clang;

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
  std::string table_varname;
  std::string iter_varname;
  std::string block_varname; 

  const FccColumn* column = &scan->m_columns[0];
  if(column->m_type == FccColumnType::E_COMPONENT)
  {
    std::string ctype = get_type_name(column->m_q_type);
    std::string q_ctype = get_qualified_type_name(column->m_q_type);
    std::string base_name = ctype;


    table_varname = generate_table_name(base_name);
    iter_varname = generate_table_iter_name(table_varname, scan);
    id++;
    block_varname = generate_block_name(base_name, scan); 
  }
  else
  {
    std::string base_name = column[0].m_ref_name;
    table_varname = generate_ref_table_name(base_name);
    iter_varname = generate_table_iter_name(table_varname, scan);
    id++;
    block_varname = generate_block_name(base_name,scan); 
  }

  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", iter_varname.c_str(), table_varname.c_str());
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());
  fprintf(p_context->p_fd, "BlockCluster %s(%s.next().get_raw());\n", block_varname.c_str(), iter_varname.c_str());

  consume(p_context->p_fd,
          scan->p_parent,
          "(&"+block_varname+")",
          scan);

  fprintf(p_context->p_fd,"}\n\n");
}

void
ProduceVisitor::visit(const Join* join)
{
  std::string hashtable = generate_hashtable_name(join);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable.c_str());
  produce(p_context->p_fd,join->p_left.get());
  produce(p_context->p_fd,join->p_right.get());
}

void
ProduceVisitor::visit(const LeftFilterJoin* left_filter_join)
{

  std::string hashtable = generate_hashtable_name(left_filter_join);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable.c_str());
  produce(p_context->p_fd,left_filter_join->p_left.get());
  produce(p_context->p_fd,left_filter_join->p_right.get());
}

void
ProduceVisitor::visit(const CrossJoin* cross_join)
{
  std::string hashtable_left = "left_" + generate_hashtable_name(cross_join);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable_left.c_str());
  produce(p_context->p_fd,cross_join->p_left.get());
  std::string hashtable_right = "right_" + generate_hashtable_name(cross_join);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable_right.c_str());
  produce(p_context->p_fd,cross_join->p_right.get());

  std::string iter_varname_left = "left_iter_hashtable_"+std::to_string(cross_join->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          iter_varname_left.c_str(), 
          hashtable_left.c_str());
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname_left.c_str());

  std::string left_cluster_name = "left_"+generate_cluster_name(cross_join);
  fprintf(p_context->p_fd,
          "BlockCluster* %s = %s.next();\n", 
          left_cluster_name.c_str(),
          iter_varname_left.c_str());

  std::string iter_varname_right = "left_iter_hashtable_"+std::to_string(cross_join->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          iter_varname_right.c_str(), 
          hashtable_right.c_str());
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname_right.c_str());

  std::string right_cluster_name = "right_"+generate_cluster_name(cross_join);
  fprintf(p_context->p_fd,
          "BlockCluster* %s = %s.next();\n", 
          left_cluster_name.c_str(),
          iter_varname_left.c_str());

  std::string joined_cluster_name = generate_cluster_name(cross_join);

  fprintf(p_context->p_fd,
          "BlockCluster %s(%s);\n", 
          joined_cluster_name.c_str(),
          left_cluster_name.c_str());

  fprintf(p_context->p_fd,
          "%s.append(%s);\n", 
          joined_cluster_name.c_str(),
          right_cluster_name.c_str());

  consume(p_context->p_fd,
          cross_join->p_parent,
          "(&"+joined_cluster_name+")",
          cross_join);


  fprintf(p_context->p_fd, 
          "}\n"); 
  fprintf(p_context->p_fd, 
          "}\n"); 
}

void
ProduceVisitor::visit(const Fetch* fetch)
{

  fprintf(p_context->p_fd, 
          "{\n"); 


  std::string global_type_name = get_type_name(fetch->m_columns[0].m_q_type);
  std::string global_var_name = generate_global_name(global_type_name,
                                                    fetch);

  std::string block_var_name = generate_cluster_name(fetch);

  fprintf(p_context->p_fd, 
          "%s* %s = database->find_global<%s>();\n", 
          global_type_name.c_str(),
          global_var_name.c_str(),
          global_type_name.c_str());

  fprintf(p_context->p_fd, 
          "if(%s != nullptr )\n{\n", 
          global_var_name.c_str());

  fprintf(p_context->p_fd, 
          "BlockCluster %s;\n", 
          block_var_name.c_str()); 

  fprintf(p_context->p_fd, 
          "%s.append_global(%s);\n", 
          block_var_name.c_str(),
          global_var_name.c_str()); 


  fprintf(p_context->p_fd,
          "%s.append(%s);\n",
          block_var_name.c_str(),
          p_context->m_source.c_str());

  consume(p_context->p_fd,
          fetch->p_parent,
          "(&"+block_var_name+")",
          fetch);

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
  std::string groups = generate_ref_groups_name(gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                gather);
  fprintf(p_context->p_fd,"BTree<DynArray<entity_id_t> > %s;\n", groups.c_str());
  produce(p_context->p_fd, gather->p_ref_table.get());

  // Generating temporal tables
  DynArray<FccColumn>& child_columns = gather->p_child.get()->m_columns;
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];

    if(child_columns[i].m_type == FccColumnType::E_REFERENCE)
    {
      StringBuilder str_builder;
      str_builder.append("<Gather> operator target component cannot be a reference column type: \"%s\"", 
                         column->m_ref_name.c_str());
      gather->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }

    if(child_columns[i].m_access_mode != FccAccessMode::E_READ)
    {
      StringBuilder str_builder;
      str_builder.append("<Gather> operator target component must have access mode READ: \"%s\"", 
                         get_type_name(column->m_q_type).c_str());
      gather->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }
    std::string component_name = get_type_name(column->m_q_type); 
    std::string temp_table_name = generate_temp_table_name(component_name, gather);
    fprintf(p_context->p_fd,"TableView<%s> %s = database->create_temp_table_no_lock<%s>(\"%s\");\n", 
            component_name.c_str(),
            temp_table_name.c_str(),
            component_name.c_str(),
            temp_table_name.c_str());

    fprintf(p_context->p_fd,"%s.clear();\n", 
            temp_table_name.c_str());
  }

  produce(p_context->p_fd, gather->p_child.get());

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string table_varname = generate_temp_table_name(component_name, gather);
    std::string iter_varname = generate_table_iter_name(table_varname);
    fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
            iter_varname.c_str(), 
            table_varname.c_str());
  }

  FccColumn* column = &child_columns[0];
  std::string component_name = get_type_name(column->m_q_type); 
  std::string table_varname = generate_temp_table_name(component_name,gather);
  std::string iter_varname = generate_table_iter_name(table_varname);
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());

  std::string cluster_varname = generate_cluster_name(gather);
  fprintf(p_context->p_fd, "BlockCluster %s;\n", cluster_varname.c_str());

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string table_varname = generate_temp_table_name(component_name, gather);
    std::string iter_varname = generate_table_iter_name(table_varname);
    fprintf(p_context->p_fd, "%s.append(%s.next().get_raw());\n", 
            cluster_varname.c_str(), 
            iter_varname.c_str());
  }

  consume(p_context->p_fd,
          gather->p_parent,
          "(&"+cluster_varname+")",
          gather);

  fprintf(p_context->p_fd,"}\n\n");

}

void
ProduceVisitor::visit(const CascadingGather* casc_gather)
{
  // GENERATING REFERENCE GROUPS
  std::string groups = generate_ref_groups_name(casc_gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                casc_gather);
  fprintf(p_context->p_fd,"BTree<DynArray<entity_id_t> > %s;\n", groups.c_str());
  produce(p_context->p_fd, casc_gather->p_ref_table.get());

  // GENERATING HASHTABLE WITH CHILD BLOCKS 
  std::string hashtable = generate_hashtable_name(casc_gather);
  fprintf(p_context->p_fd,"BTree<BlockCluster> %s;\n", hashtable.c_str());
  produce(p_context->p_fd, casc_gather->p_child.get());


  // CREATE TEMPORAL TABLES
  DynArray<FccColumn>& child_columns = casc_gather->p_child.get()->m_columns;
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];

    if(child_columns[i].m_type == FccColumnType::E_REFERENCE)
    {
      StringBuilder str_builder;
      str_builder.append("<Gather> operator target component cannot be a reference column type: \"%s\"", 
                         column->m_ref_name.c_str());
      casc_gather->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }

    if(child_columns[i].m_access_mode != FccAccessMode::E_READ)
    {
      StringBuilder str_builder;
      str_builder.append("<Gather> operator target component must have access mode READ: \"%s\"", 
                         get_type_name(column->m_q_type).c_str());
      casc_gather->p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }
    std::string component_name = get_type_name(column->m_q_type); 
    std::string temp_table_name = generate_temp_table_name(component_name, casc_gather);
    fprintf(p_context->p_fd,"TableView<%s> %s = database->create_temp_table_no_lock<%s>(\"%s\");\n", 
            component_name.c_str(),
            temp_table_name.c_str(),
            component_name.c_str(),
            temp_table_name.c_str());
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
          groups.c_str(),
          casc_gather->m_id);

  fprintf(p_context->p_fd,
          "while(current_frontier_%u->size() > 0)\n{\n", 
          casc_gather->m_id);

  // CLEARING TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string table_varname = generate_temp_table_name(component_name, casc_gather);

    fprintf(p_context->p_fd,"%s.clear();\n", 
            table_varname.c_str());
  }

  // FILLING UP TEMPORAL TABLES
  std::string iter_varname = "iter_hashtable_"+std::to_string(casc_gather->m_id);
  fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
          iter_varname.c_str(), 
          hashtable.c_str());

  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());
  fprintf(p_context->p_fd,
          "gather(&%s,%s.next().p_value,current_frontier_%u, next_frontier_%u",
          groups.c_str(),
          iter_varname.c_str(),
          casc_gather->m_id,
          casc_gather->m_id);
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string temp_table_name = generate_temp_table_name(component_name, casc_gather);
    fprintf(p_context->p_fd,",&%s",
            temp_table_name.c_str());
  }
  fprintf(p_context->p_fd,");\n");
  fprintf(p_context->p_fd,"}\n");


  // ITERATE OVER TEMPORAL TABLES
  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string table_varname = generate_temp_table_name(component_name, casc_gather);

    std::string iter_varname = generate_table_iter_name(table_varname);
    fprintf(p_context->p_fd, "auto %s = %s.iterator();\n", 
            iter_varname.c_str(), 
            table_varname.c_str());
  }

  FccColumn* column = &child_columns[0];
  std::string component_name = get_type_name(column->m_q_type); 
  std::string table_varname = generate_temp_table_name(component_name,casc_gather);
  iter_varname = generate_table_iter_name(table_varname);
  fprintf(p_context->p_fd, "while(%s.has_next())\n{\n", iter_varname.c_str());

  std::string cluster_varname = generate_cluster_name(casc_gather);
  fprintf(p_context->p_fd, "BlockCluster %s;\n", cluster_varname.c_str());

  for(uint32_t i = 0; i < child_columns.size(); ++i)
  {
    FccColumn* column = &child_columns[i];
    std::string component_name = get_type_name(column->m_q_type); 
    std::string table_varname = generate_temp_table_name(component_name, casc_gather);
    std::string iter_varname = generate_table_iter_name(table_varname);
    fprintf(p_context->p_fd, "%s.append(%s.next().get_raw());\n", 
            cluster_varname.c_str(), 
            iter_varname.c_str());
  }

  consume(p_context->p_fd,
          casc_gather->p_parent,
          "(&"+cluster_varname+")",
          casc_gather);

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
