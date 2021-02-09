
#include "../driver.h"
#include "../fcc_context.h"
#include "../frontend/exec_plan_printer.h"
#include "../frontend/operator.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "producer.h"
#include "reflection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
fcc_generate_task_graph(FILE* fd, 
                        const char* task_graph_name,
                        const char* task_prefix,
                        const fcc_exec_plan_t* exec_plan)
{
  uint32_t num_tasks = exec_plan->m_nnodes;
  for(uint32_t i = 0; i < num_tasks; ++i)
  {
    fprintf(fd, 
            "fdb_task_graph_insert_task(%s,%d,%s_%d, (bool)%d, %s_%d_info);\n", 
            task_graph_name,
            i,
            task_prefix,
            i,
            exec_plan->m_subplans[i]->m_requires_sync,
            task_prefix,
            i
           );
  }

  for(uint32_t i = 0; i < num_tasks; ++i)
  {
    fcc_exec_plan_node_t* node = &exec_plan->m_nodes[i];
    uint32_t num_children = node->m_parents.m_count;
    uint32_t* parents = node->m_parents.m_data;
    for(uint32_t j = 0; j < num_children; ++j)
    {
      fprintf(fd, 
              "fdb_task_graph_set_parent(%s,%d,%d);\n", 
              task_graph_name,
              i,
              parents[j]);
    }
  }

  uint32_t num_roots = exec_plan->m_nroots;
  for(uint32_t i = 0; i < num_roots; ++i)
  {
    fprintf(fd, 
            "fdb_task_graph_set_root(%s,%d,%d);\n", 
            task_graph_name,
            i,
            exec_plan->m_roots[i]);
  }
}

int32_t 
fcc_generate_code(const fcc_exec_plan_t* exec_plan,
                  const fcc_exec_plan_t* post_exec_plan,
                  const char* filename,
                  const char* include_file)
{
  uint32_t code_buffer_length=4096;
  char* code_buffer = new char[code_buffer_length];
  FILE* fd = fopen(filename, "w");
  fprintf(fd, "\n\n\n");
  // ADDING FURIOUS INCLUDE 
  fprintf(fd, "#include <%s> \n", include_file);

  // LOOKING FOR DEPENDENCIES 
  fcc_deps_extr_t deps_extr;
  fcc_deps_extr_init(&deps_extr);
  uint32_t num_nodes = exec_plan->m_nnodes;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    fcc_deps_extr_extract(&deps_extr, exec_plan->m_subplans[i]);
  }

  num_nodes = post_exec_plan->m_nnodes;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    fcc_deps_extr_extract(&deps_extr, post_exec_plan->m_subplans[i]);
  }

  // ADDING REQUIRED INCLUDES
  const uint32_t num_includes = deps_extr.m_incs.m_count;
  const char** incs = deps_extr.m_incs.m_data;
  for (uint32_t i = 0; i < num_includes; ++i) 
  {
    fprintf(fd, "#include \"%s\"\n", incs[i]);
  }

  fprintf(fd, "\n\n\n");

  // ADDING REQUIRED "USING NAMESPACE DIRECTIVES TODO: Add support for other
  // using clauses"
  fcc_decl_t* usings = p_fcc_context->m_using_decls.m_data;
  const uint32_t num_usings = p_fcc_context->m_using_decls.m_count;
  for(uint32_t i = 0; i < num_usings; ++i)
  {
    uint32_t length = 0;
    while ((length = fcc_decl_code(usings[i], code_buffer, code_buffer_length)) >= code_buffer_length)
    {
      delete [] code_buffer;
      code_buffer_length*=2;
      code_buffer = new char[code_buffer_length];
    }
    fprintf(fd,"%s;\n",code_buffer);
  }

  // ADDING DECLARATIONS FOUND IN FURIOUS SCRIPTS
  const uint32_t num_decls = deps_extr.m_decls.m_count;
  fcc_decl_t* decls = deps_extr.m_decls.m_data;
  for (uint32_t i = 0; i < num_decls; ++i) 
  {
    uint32_t length = 0;
    while ((length = fcc_decl_code(decls[i], code_buffer, code_buffer_length)) >= code_buffer_length)
    {
      delete [] code_buffer;
      code_buffer_length*=2;
      code_buffer = new char[code_buffer_length];
    }
    fprintf(fd,"%s;\n\n", code_buffer);
  }
  fcc_deps_extr_release(&deps_extr);

  // STARTING CODE GENERATION

  /// DECLARE VARIABLES (e.g. TABLEVIEWS, BITTABLES, etc.)
  fprintf(fd, "\n\n\n");
  fprintf(fd,"// Variable declarations \n");

  fprintf(fd,"fdb_htregistry_t ht_registry;\n");

  fcc_vars_extr_t vars_extr;
  fcc_vars_extr_init(&vars_extr);
  num_nodes = exec_plan->m_nnodes;
  for (uint32_t i = 0; i < num_nodes; ++i) 
  {
    fcc_vars_extr_extract(&vars_extr, exec_plan->m_subplans[i]);
  }

  num_nodes = post_exec_plan->m_nnodes;
  for (uint32_t i = 0; i < num_nodes; ++i) 
  {
    fcc_vars_extr_extract(&vars_extr, post_exec_plan->m_subplans[i]);
  }

  // DECLARING TABLEVIEWS
  const uint32_t ncomps = vars_extr.m_comps.m_count;
  const char** comps = vars_extr.m_comps.m_data;
  for (uint32_t i = 0; i < ncomps; ++i) 
  {
    char tmp[FCC_MAX_TABLE_VARNAME];
    generate_table_name(comps[i], 
                        tmp,
                        FCC_MAX_TABLE_VARNAME);


    fprintf(fd, "struct fdb_txtable_t* %s;\n", tmp);
  }

  const uint32_t nrefs = vars_extr.m_refs.m_count;
  const char** refs = vars_extr.m_refs.m_data;
  for (uint32_t i = 0; i < nrefs; ++i) 
  {
    char tmp[FCC_MAX_REF_TABLE_VARNAME];
    generate_ref_table_name(refs[i], 
                            tmp, 
                            FCC_MAX_REF_TABLE_VARNAME);

    fprintf(fd, "struct fdb_txtable_t* %s;\n", tmp);
  }

  // DECLARING BITTABLES
  const uint32_t num_tags = vars_extr.m_tags.m_count;
  const char** tags = vars_extr.m_tags.m_data;
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    char tmp[FCC_MAX_TAG_TABLE_VARNAME];
    generate_bittable_name(tags[i],
                           tmp,
                           FCC_MAX_TAG_TABLE_VARNAME);


    fprintf(fd, "struct fdb_txbittable_t* %s;\n", tmp);
  }

  // DECLARING SYSTEMWRAPPERS
  fcc_stmt_t** stmts = p_fcc_context->m_stmts.m_data;
  uint32_t nstmts = p_fcc_context->m_stmts.m_count;
  for(uint32_t i = 0; i < nstmts; ++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[FCC_MAX_TYPE_NAME];
    fcc_type_name(match->p_system->m_system_type, 
                  system_name, 
                  FCC_MAX_TYPE_NAME);

    char wrapper_name[FCC_MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 match->p_system->m_id,
                                 wrapper_name, 
                                 FCC_MAX_SYSTEM_WRAPPER_VARNAME);


    fprintf(fd, "%s* %s;\n", system_name, wrapper_name);
  }

  fprintf(fd, "struct fdb_task_graph_t task_graph;\n");
  fprintf(fd, "struct fdb_task_graph_t post_task_graph;\n");

  // DEFINING TASKS CODE BASED ON EXECUTION PLAN ROOTS
  num_nodes = exec_plan->m_nnodes;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    fcc_subplan_printer_t printer;
    fcc_subplan_printer_init(&printer, true, false);
    fcc_subplan_printer_print(&printer, 
                              exec_plan->m_subplans[i]);
    fprintf(fd,"const char* __task_%d_info = \"%s\";\n", i, printer.m_str_builder.p_buffer);
    fprintf(fd,"void __task_%d(struct fdb_tx_t* tx,\n\
            struct fdb_txthread_ctx_t* txtctx,\n\
            float delta,\n\
            struct fdb_database_t* database,\n\
            void* user_data,\n\
            uint32_t chunk_size,\n\
            uint32_t offset,\n\
            uint32_t stride,\n\
            struct fdb_barrier_t* barrier)\n", 
            i);

    fprintf(fd,"{\n");

    fprintf(fd, "struct fdb_context_t context;\n");
    fprintf(fd, "fdb_context_init(&context, delta,database,user_data, chunk_size, offset, stride);\n");
    fprintf(fd, "struct fdb_stack_alloc_t task_allocator;\n");
    fprintf(fd, "fdb_stack_alloc_init(&task_allocator, KILOBYTES(4), fdb_get_global_mem_allocator());\n");

  // INITIALIZING  BTREE FACTORY
    fprintf(fd,"struct fdb_btree_factory_t btree_factory;\n\n");
    fprintf(fd, "fdb_btree_factory_init(&btree_factory\n, &task_allocator.m_super);\n");

  // INITIALIZING  BCLUSTER FACTORY
    fprintf(fd,"struct fdb_bcluster_factory_t bcluster_factory;\n\n");
    fprintf(fd, "fdb_bcluster_factory_init(&bcluster_factory\n, &task_allocator.m_super);\n");

    // INITIALIZING BITMAP FACTORY
    fprintf(fd,"struct fdb_bitmap_factory_t bitmap_factory;\n\n");
    fprintf(fd, "fdb_bitmap_factory_init(&bitmap_factory\n, FDB_TXTABLE_BLOCK_SIZE, &task_allocator.m_super);\n");

    // INITIALIZING TMPTABLE FACTORY
    fprintf(fd,"struct fdb_tmptable_factory_t tmptable_factory;\n\n");
    fprintf(fd, "fdb_tmptable_factory_init(&tmptable_factory\n, &task_allocator.m_super);\n");

    // INITIALIZING TMPBITTABLE FACTORY
    fprintf(fd,"struct fdb_tmpbittable_factory_t tmpbittable_factory;\n\n");
    fprintf(fd, "fdb_tmpbittable_factory_init(&tmpbittable_factory\n, &task_allocator.m_super);\n");

    const fcc_operator_t* root = &exec_plan->m_subplans[i]->m_nodes[exec_plan->m_subplans[i]->m_root];
    produce(fd,root, true);

    // RELEASING TMPBITTABLE FACTORY 
    fprintf(fd, 
            "fdb_tmpbittable_factory_release(&tmpbittable_factory);\n\n");

    // RELEASING TMPTABLE FACTORY 
    fprintf(fd, 
            "fdb_tmptable_factory_release(&tmptable_factory);\n\n");

    // RELEASING BCLUSTER FACTORY 
    fprintf(fd, 
            "fdb_bitmap_factory_release(&bitmap_factory);\n\n");

    // RELEASING BCLUSTER FACTORY 
    fprintf(fd, 
            "fdb_bcluster_factory_release(&bcluster_factory);\n\n");

    // RELEASING BTREE FACTORY 
    fprintf(fd, 
            "fdb_btree_factory_release(&btree_factory);\n\n");
    fprintf(fd, "fdb_stack_alloc_release(&task_allocator);\n");
    fprintf(fd,"}\n");
    fcc_subplan_printer_release(&printer);
  }

  // DEFINING TASKS CODE BASED ON POST FRAME EXECUTION PLAN ROOTS
  num_nodes = post_exec_plan->m_nnodes;
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    fcc_subplan_printer_t printer;
    fcc_subplan_printer_init(&printer, true, false);
    fcc_subplan_printer_print(&printer, 
                              post_exec_plan->m_subplans[i]);
    fprintf(fd,"const char* __pf_task_%d_info = \"%s\";\n", i, printer.m_str_builder.p_buffer);
    fprintf(fd,"void __pf_task_%d(struct fdb_tx_t* tx,\n\
            struct fdb_txthread_ctx_t* txtctx,\n\
            float delta,\n\
    struct fdb_database_t* database,\n\
            void* user_data,\n\
            uint32_t chunk_size,\n\
            uint32_t offset,\n\
            uint32_t stride,\n\
            struct fdb_barrier_t* barrier)\n", i);
    fprintf(fd,"{\n");

    fprintf(fd, "struct fdb_context_t context;\n");
    fprintf(fd, "struct fdb_stack_alloc_t task_allocator;\n");
    fprintf(fd, "fdb_stack_alloc_init(&task_allocator, KILOBYTES(4), fdb_get_global_mem_allocator());\n");
    const fcc_operator_t* root = &post_exec_plan->m_subplans[i]->m_nodes[post_exec_plan->m_subplans[i]->m_root];
    produce(fd,root, true);
    fprintf(fd, "fdb_stack_alloc_release(&task_allocator);\n");
    fprintf(fd,"}\n");
    fcc_subplan_printer_release(&printer);
  }

  /// GENERATING furious__init  
  fprintf(fd, "\n\n\n");
  fprintf(fd, "// Variable initializations \n");
  fprintf(fd, "void furious_init(struct fdb_database_t* database)\n{\n");

  // CREATING INIT TRANSACTION
  fprintf(fd, "struct fdb_tx_t tx;");
  fprintf(fd, "fdb_tx_begin(&tx, E_READ_WRITE);");
  fprintf(fd, "struct fdb_txthread_ctx_t* txtctx = fdb_tx_txthread_ctx_get(&tx, NULL);");

  // INITIALIZING HT REGISTRY
  fprintf(fd, "fdb_htregistry_init(&ht_registry\n, fdb_get_global_mem_allocator());\n");


  // INITIALIZING TABLEVIEWS 
  {
    const uint32_t ncomps = vars_extr.m_comps.m_count;
    const char**  comps = vars_extr.m_comps.m_data;
    for (uint32_t i = 0; i < ncomps; ++i) 
    {
      char tmp[FCC_MAX_TABLE_VARNAME];
      generate_table_name(comps[i],
                          tmp,
                          FCC_MAX_TABLE_VARNAME);

      fprintf(fd,
              "%s  = FDB_FIND_OR_CREATE_TABLE(database, &tx, txtctx, %s, nullptr);\n",
              tmp,
              comps[i]);
    }
  }

  {
    const uint32_t nrefs = vars_extr.m_refs.m_count;
    const char** refs = vars_extr.m_refs.m_data;
    for (uint32_t i = 0; i < nrefs; ++i) 
    {

      char tmp[FCC_MAX_REF_TABLE_VARNAME];
      generate_ref_table_name(refs[i], 
                              tmp, 
                              FCC_MAX_REF_TABLE_VARNAME);


      fprintf(fd,
              "%s  = FDB_FIND_OR_CREATE_REF_TABLE(database, &tx, txtctx, \"%s\");\n",
              tmp,
              refs[i]);
    }
  }

  // INITIALIZING BITTABLES
  {
    const uint32_t ntags = vars_extr.m_tags.m_count;
    const char** tags = vars_extr.m_tags.m_data;
    for (uint32_t i = 0; i < ntags; ++i) 
    {
      char tmp[FCC_MAX_TAG_TABLE_VARNAME];
      generate_bittable_name(tags[i], 
                             tmp, 
                             FCC_MAX_TAG_TABLE_VARNAME);


      fprintf(fd,
              "%s = FDB_FIND_TAG_TABLE(database, &tx, txtctx, \"%s\");\n",
              tmp,
              tags[i]);
    }
  }

  // INITIALIZING SYSTEM WRAPPERS
  for(uint32_t i = 0; i < nstmts; ++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[FCC_MAX_TYPE_NAME];
    fcc_type_name(match->p_system->m_system_type, 
                  system_name, 
                  FCC_MAX_TYPE_NAME);


    char wrapper_name[FCC_MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 match->p_system->m_id,
                                 wrapper_name, 
                                 FCC_MAX_SYSTEM_WRAPPER_VARNAME);


    fprintf(fd,
            "%s = new %s(",
            wrapper_name,
            system_name);

    fcc_expr_t* ctor_params = match->p_system->m_ctor_params;
    uint32_t ncparams = match->p_system->m_nctor_params;
    if(ncparams > 0) 
    {
      fcc_expr_t expr = ctor_params[0];
      uint32_t length = 0;
      while ((length = fcc_expr_code(expr, code_buffer, code_buffer_length)) >= code_buffer_length)
      {
        delete [] code_buffer;
        code_buffer_length*=2;
        code_buffer = new char[code_buffer_length];
      }
      fprintf(fd,"%s", code_buffer);

      for(size_t i = 1; i < ncparams; ++i)
      {
        fcc_expr_t expr = ctor_params[i];
        while ((length = fcc_expr_code(expr, code_buffer, code_buffer_length)) >= code_buffer_length)
        {
          delete [] code_buffer;
          code_buffer_length*=2;
          code_buffer = new char[code_buffer_length];
        }
        fprintf(fd,",%s",code_buffer);
      }
    }
    fprintf(fd,");\n");
  }

  // GENERATING REFLECTION CODE
  {
    const uint32_t num_decls = vars_extr.m_comp_decls.m_count;
    fcc_decl_t* decls = vars_extr.m_comp_decls.m_data;
    for (uint32_t i = 0; i < num_decls; ++i) 
    {
      generate_reflection_code(fd, decls[i]);
    }
  }
  fcc_vars_extr_release(&vars_extr);

  fprintf(fd, 
          "fdb_task_graph_init(&task_graph, %d, %d);\n", 
          exec_plan->m_nnodes,
          exec_plan->m_nroots);

  fcc_generate_task_graph(fd, "&task_graph", "__task", exec_plan);

  fprintf(fd, 
          "fdb_task_graph_init(&post_task_graph, %d, %d);\n", 
          post_exec_plan->m_nnodes,
          post_exec_plan->m_nroots);

  fcc_generate_task_graph(fd, "&post_task_graph", "__pf_task", post_exec_plan);

  fprintf(fd, "fdb_tx_commit(&tx);");

  fprintf(fd,"}\n");

  /// GENERATING furious_frame CODE
  {
    fprintf(fd,"\n\n\n");
    fprintf(fd,"void furious_frame(float delta, fdb_database_t* database, void* user_data)\n{\n");

    fprintf(fd, "fdb_task_graph_run(&task_graph, delta, database, user_data);\n");

    fprintf(fd, "}\n");
  }


  /// GENERATING furious_post_frame CODE
  {
    fprintf(fd,"\n\n\n");
    fprintf(fd,"void furious_post_frame(float delta, fdb_database_t* database, void* user_data)\n{\n");

    fprintf(fd, "fdb_task_graph_run(&post_task_graph, delta, database, user_data);\n");

    fprintf(fd, "}\n");
  }

  // GENERATING furious_release CODE
  fprintf(fd, "// Variable releases \n");
  fprintf(fd, "void furious_release()\n{\n");

  for(uint32_t i = 0; i < nstmts; ++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[FCC_MAX_TYPE_NAME];
    fcc_type_name(match->p_system->m_system_type, 
                  system_name, 
                  FCC_MAX_TYPE_NAME);

    char wrapper_name[FCC_MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 match->p_system->m_id,
                                 wrapper_name, 
                                 FCC_MAX_SYSTEM_WRAPPER_VARNAME);



    fprintf(fd, "delete %s;\n", wrapper_name);
  }

  fprintf(fd, 
          "fdb_task_graph_release(&task_graph);\n");

  fprintf(fd, 
          "fdb_task_graph_release(&post_task_graph);\n");


  // RELEASING HT REGISTRY
  fprintf(fd, 
          "fdb_htregistry_release(&ht_registry);\n\n");

  fprintf(fd, "}\n");

  fprintf(fd,
          "fdb_task_graph_t* furious_task_graph()\n{\n");
  fprintf(fd, 
          "return &task_graph;\n");
  fprintf(fd, 
          "}\n");

  fprintf(fd,
          "fdb_task_graph_t* furious_post_task_graph()\n{\n");
  fprintf(fd, 
          "return &post_task_graph;\n");
  fprintf(fd, 
          "}\n");

  fclose(fd);
  delete [] code_buffer;
  return 0;
} 
