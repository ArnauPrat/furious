

#include "../driver.h"
#include "../drivers/clang/clang_tools.h"
#include "../common/dyn_array.h"
#include "../fcc_context.h"
#include "../frontend/exec_plan_printer.h"
#include "../frontend/exec_plan.h"
#include "../frontend/operator.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "reflection.h"

#include <stdio.h>
#include <stdlib.h>
#include <set>

namespace furious 
{

CodeGenRegistry* p_registry = nullptr;;

/**
 * @brief Visitor used to extract the dependencies of an execution plan, which
 * include structures and headers including dependent structures
 */
class DependenciesExtr : public FccSubPlanVisitor 
{
public:

  DynArray<char*>   m_include_files;
  DynArray<fcc_decl_t>    m_declarations;

  ~DependenciesExtr()
  {
    const uint32_t num_includes = m_include_files.size();
    for (uint32_t i = 0; i < num_includes; ++i) 
    {
      delete [] m_include_files[i];
    }
  }

  void
  extract_dependencies(const DynArray<Dependency>& deps)
  {
    for(uint32_t i = 0; i < deps.size(); ++i) 
    {
      const Dependency& dep = deps[i];
      if(fcc_decl_is_valid(dep.m_decl))
      {
        if(fcc_decl_is_variable_or_struct(dep.m_decl) || 
           fcc_decl_is_function(dep.m_decl))
        {
          bool found = false;
          const uint32_t num_decls = m_declarations.size();
          for (uint32_t i = 0; i < num_decls; ++i) 
          {
            if(fcc_decl_is_same(dep.m_decl, m_declarations[i]))
            {
              found = true;
              break;
            }
          }

          if(!found)
          {
            m_declarations.append(dep.m_decl);
          }
        }
      } 
      else 
      {
        char* buffer = new char[MAX_INCLUDE_PATH_LENGTH];
        strncpy(buffer, dep.m_include_file.c_str(), MAX_INCLUDE_PATH_LENGTH);
        FURIOUS_CHECK_STR_LENGTH(strlen(dep.m_include_file.c_str()), MAX_INCLUDE_PATH_LENGTH);
        m_include_files.append(buffer);
      }
    }
  }

  virtual void 
  visit(const Foreach* foreach)
  {
    uint32_t size = foreach->p_systems.size();
    for(uint32_t i = 0; i < size; ++i)
    {
      const fcc_system_t* system = foreach->p_systems[i];
      extract_dependencies(fcc_type_dependencies(system->m_system_type));
    }
    foreach->p_child.get()->accept(this);
  }

  virtual void 
  visit(const Scan* scan) 
  {
    if(scan->m_columns[0].m_type == FccColumnType::E_COMPONENT)
    {
      extract_dependencies(fcc_type_dependencies(scan->m_columns[0].m_component_type));
    }
  } 

  virtual void
  visit(const Join* join) 
  {
    join->p_left.get()->accept(this);
    join->p_right.get()->accept(this);
  }

  virtual void
  visit(const LeftFilterJoin* left_filter_join) 
  {
    left_filter_join->p_left.get()->accept(this);
    left_filter_join->p_right.get()->accept(this);
  }

  virtual void
  visit(const CrossJoin* cross_join) 
  {
    cross_join->p_left.get()->accept(this);
    cross_join->p_right.get()->accept(this);
  }

  virtual void
  visit(const Fetch* fetch) 
  {
    if(fetch->m_columns[0].m_type == FccColumnType::E_GLOBAL)
    {
      extract_dependencies(fcc_type_dependencies(fetch->m_columns[0].m_component_type));
    }
  }

  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    tag_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    extract_dependencies(fcc_type_dependencies(component_filter->m_filter_type));
    component_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const PredicateFilter* predicate_filter) 
  {
    extract_dependencies(fcc_decl_dependencies(predicate_filter->m_func_decl));
    predicate_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const Gather* gather) 
  {
    gather->p_child.get()->accept(this);
  }

  virtual void
  visit(const CascadingGather* casc_gather) 
  {
    casc_gather->p_child.get()->accept(this);
  }
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * \brief Extracts the components and tags used in an execution plan, which will
 * be used to declare the required variables in the generated code.
 */
class VarsExtr : public FccSubPlanVisitor 
{

public:
  DynArray<fcc_decl_t>    m_component_decls;
  DynArray<char*>     m_tags;
  DynArray<char*>     m_references;
  DynArray<char*>     m_components;

  ~VarsExtr()
  {
    const uint32_t num_components = m_components.size();
    for (uint32_t i = 0; i < num_components; ++i) 
    {
      delete [] m_components[i];
    }

    const uint32_t num_tags = m_tags.size();
    for (uint32_t i = 0; i < num_tags; ++i) 
    {
      delete [] m_tags[i];
    }

    const uint32_t num_refs = m_references.size();
    for (uint32_t i = 0; i < num_refs; ++i) 
    {
      delete [] m_references[i];
    }
  }


  virtual void 
  visit(const Foreach* foreach)
  {
    foreach->p_child.get()->accept(this);
  }

  virtual void 
  visit(const Scan* scan) 
  {
    if(scan->m_columns[0].m_type == FccColumnType::E_COMPONENT)
    {
      char tmp[MAX_TYPE_NAME];
      const uint32_t length = fcc_type_name(scan->m_columns[0].m_component_type, 
                                            tmp, MAX_TYPE_NAME);
      FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

      const uint32_t num_components = m_components.size();
      bool found = false;
      for(uint32_t i = 0; i < num_components; ++i)
      {
        if(strcmp(tmp, m_components[i]) == 0)
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        char* buffer = new char [MAX_TYPE_NAME];
        strncpy(buffer, tmp, MAX_TYPE_NAME);
        m_components.append(buffer);
        fcc_decl_t decl;
        fcc_type_decl(scan->m_columns[0].m_component_type, &decl);
        bool found = false;
        const uint32_t num_decls = m_component_decls.size();
        for (uint32_t i = 0; i < num_decls; ++i) 
        {
          if(fcc_decl_is_same(m_component_decls[i], decl))
          {
            found = true;
            break;
          }
        }
        if(!found)
        {
          m_component_decls.append(decl);
        }
      }
    }
    else
    {
      char* buffer = new char[MAX_REF_NAME];
      strncpy(buffer, scan->m_columns[0].m_ref_name, MAX_REF_NAME);
      m_references.append(buffer);
    }
  }

  virtual void
  visit(const Join* join) 
  {
    join->p_left.get()->accept(this);
    join->p_right.get()->accept(this);
  }

  virtual void
  visit(const LeftFilterJoin* left_filter_join) 
  {
    left_filter_join->p_left.get()->accept(this);
    left_filter_join->p_right.get()->accept(this);
  }

  virtual void
  visit(const CrossJoin* cross_join) 
  {
    cross_join->p_left.get()->accept(this);
    cross_join->p_right.get()->accept(this);
  }

  virtual void
  visit(const Fetch* fetch)
  {
      fcc_decl_t decl;
      fcc_type_decl(fetch->m_columns[0].m_component_type, &decl);
      bool found = false;
      const uint32_t num_decls = m_component_decls.size();
      for (uint32_t i = 0; i < num_decls; ++i) 
      {
        if(fcc_decl_is_same(decl, m_component_decls[i]))
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        m_component_decls.append(decl);
      }
  }

  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    char* buffer = new char[MAX_TAG_NAME];
    strncpy(buffer, tag_filter->m_tag, MAX_TAG_NAME);
    m_tags.append(buffer);
    tag_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    char tmp[MAX_TYPE_NAME];
    const uint32_t length = fcc_type_name(component_filter->m_filter_type, tmp, MAX_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    bool found = false;
    const uint32_t num_components = m_components.size();
    for(uint32_t i = 0; i < num_components; ++i)
    {
      if(strcmp(m_components[i], tmp) == 0)
      {
        found = true;
      }
    }

    if(!found)
    {
      char* buffer = new char[MAX_TYPE_NAME];
      strncpy(buffer, tmp, MAX_TYPE_NAME);
      m_components.append(buffer);
      fcc_decl_t decl;
      fcc_type_decl(component_filter->m_filter_type, &decl);
      m_component_decls.append(decl);
      component_filter->p_child.get()->accept(this);
    }
  }

  virtual void
  visit(const PredicateFilter* predicate_filter) 
  {
    predicate_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const Gather* gather)
  {
    gather->p_ref_table.get()->accept(this);
    gather->p_child.get()->accept(this);
  }

  virtual void
  visit(const CascadingGather* casc_gather)
  {
    casc_gather->p_ref_table.get()->accept(this);
    casc_gather->p_child.get()->accept(this);
  }
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

int32_t 
fcc_generate_code(const FccExecPlan* exec_plan,
                  const FccExecPlan* post_exec_plan,
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
  DependenciesExtr deps_visitor;
  uint32_t num_nodes = exec_plan->m_subplans.size();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    deps_visitor.traverse(&exec_plan->m_subplans[i]);
  }

  num_nodes = post_exec_plan->m_subplans.size();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    deps_visitor.traverse(&post_exec_plan->m_subplans[i]);
  }

  // ADDING REQUIRED INCLUDES
  const uint32_t num_includes = deps_visitor.m_include_files.size();
  for (uint32_t i = 0; i < num_includes; ++i) 
  {
    fprintf(fd, "#include \"%s\"\n", deps_visitor.m_include_files[i]);
  }

  fprintf(fd, "\n\n\n");

  // ADDING REQUIRED "USING NAMESPACE DIRECTIVES TODO: Add support for other
  // using clauses"
  const DynArray<fcc_decl_t>& usings = p_fcc_context->m_using_decls;
  const uint32_t num_usings = usings.size();
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
  const uint32_t num_decls = deps_visitor.m_declarations.size();
  for (uint32_t i = 0; i < num_decls; ++i) 
  {
    uint32_t length = 0;
    while ((length = fcc_decl_code(deps_visitor.m_declarations[i], code_buffer, code_buffer_length)) >= code_buffer_length)
    {
      delete [] code_buffer;
      code_buffer_length*=2;
      code_buffer = new char[code_buffer_length];
    }
    fprintf(fd,"%s;\n\n", code_buffer);
  }

  // STARTING CODE GENERATION
  fprintf(fd,"namespace furious \n{\n");

  /// DECLARE VARIABLES (e.g. TABLEVIEWS, BITTABLES, etc.)
  fprintf(fd, "\n\n\n");
  fprintf(fd,"// Variable declarations \n");
  VarsExtr vars_extr;
  num_nodes = exec_plan->m_subplans.size();
  for (uint32_t i = 0; i < num_nodes; ++i) 
  {
    vars_extr.traverse(&exec_plan->m_subplans[i]);
  }

  num_nodes = post_exec_plan->m_nodes.size();
  for (uint32_t i = 0; i < num_nodes; ++i) 
  {
    vars_extr.traverse(&post_exec_plan->m_subplans[i]);
  }

  // DECLARING TABLEVIEWS
  const uint32_t num_components = vars_extr.m_components.size();
  for (uint32_t i = 0; i < num_components; ++i) 
  {
    char tmp[MAX_TABLE_VARNAME];
    const uint32_t length = generate_table_name(vars_extr.m_components[i], 
                                                tmp,
                                                MAX_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

    fprintf(fd, "TableView<%s> %s;\n", vars_extr.m_components[i], tmp);
  }

  const uint32_t num_references = vars_extr.m_references.size();
  for (uint32_t i = 0; i < num_references; ++i) 
  {
    char tmp[MAX_REF_TABLE_VARNAME];
    const uint32_t length = generate_ref_table_name(vars_extr.m_references[i], 
                                                    tmp, 
                                                    MAX_REF_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_REF_TABLE_VARNAME);

    fprintf(fd, "TableView<entity_id_t> %s;\n", tmp);
  }

  // DECLARING BITTABLES
  const uint32_t num_tags = vars_extr.m_tags.size();
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    char tmp[MAX_TAG_TABLE_VARNAME];
    const uint32_t length = generate_bittable_name(vars_extr.m_tags[i],
                                                   tmp,
                                                   MAX_TAG_TABLE_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TAG_TABLE_VARNAME);

    fprintf(fd, "BitTable* %s;\n", tmp);
  }

  // DECLARING SYSTEMWRAPPERS
  const DynArray<fcc_stmt_t*>& stmts = p_fcc_context->p_stmts;
  for(uint32_t i = 0; i < stmts.size();++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[MAX_TYPE_NAME];
    const uint32_t system_length = fcc_type_name(match->p_system->m_system_type, 
                  system_name, 
                  MAX_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(system_length, MAX_TYPE_NAME);

    char wrapper_name[MAX_SYSTEM_WRAPPER_VARNAME];
    const uint32_t wrapper_length = generate_system_wrapper_name(system_name, 
                                                                 match->p_system->m_id,
                                                                 wrapper_name, 
                                                                 MAX_SYSTEM_WRAPPER_VARNAME);
    FURIOUS_CHECK_STR_LENGTH(wrapper_length, MAX_SYSTEM_WRAPPER_VARNAME);


    fprintf(fd, "%s* %s;\n", system_name, wrapper_name);
  }

  // DEFINING TASKS CODE BASED ON EXECUTION PLAN ROOTS
  p_registry = new CodeGenRegistry();
  num_nodes = exec_plan->m_subplans.size();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    const FccOperator* root = exec_plan->m_subplans[i].p_root;
    ExecPlanPrinter printer(true);
    root->accept(&printer);
    fprintf(fd,"%s", printer.m_str_builder.p_buffer);
    fprintf(fd,"void __task_%d(float delta,\n\
    Database* database,\n\
            void* user_data,\n\
            uint32_t chunk_size,\n\
            uint32_t offset,\n\
            uint32_t stride)\n", 
            i);

    fprintf(fd,"{\n");

    fprintf(fd, "Context context(delta,database,user_data);\n");
    produce(fd,root);
    //fprintf(fd,"database->remove_temp_tables_no_lock();\n");
    fprintf(fd,"}\n");
  }
  delete p_registry;

  /// GENERATING __furious__init  
  fprintf(fd, "\n\n\n");
  fprintf(fd, "// Variable initializations \n");
  fprintf(fd, "void __furious_init(Database* database)\n{\n");

  // INITIALIZING TABLEVIEWS 
  {
    const uint32_t num_components = vars_extr.m_components.size();
    for (uint32_t i = 0; i < num_components; ++i) 
    {
      char tmp[MAX_TABLE_VARNAME];
      const uint32_t length = generate_table_name(vars_extr.m_components[i],
                                                  tmp,
                                                  MAX_TABLE_VARNAME);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

      fprintf(fd,
              "%s  = FURIOUS_FIND_OR_CREATE_TABLE(database, %s);\n",
              tmp,
              vars_extr.m_components[i]);
    }
  }

  {
    const uint32_t num_references = vars_extr.m_references.size();
    for (uint32_t i = 0; i < num_references; ++i) 
    {

      char tmp[MAX_REF_TABLE_VARNAME];
      const uint32_t length = generate_ref_table_name(vars_extr.m_references[i], 
                                                      tmp, 
                                                      MAX_REF_TABLE_VARNAME);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_REF_TABLE_VARNAME);

      fprintf(fd,
              "%s  = database->get_references(\"%s\");\n",
              tmp,
              vars_extr.m_references[i]);
    }
  }

  // INITIALIZING BITTABLES
  {
    const uint32_t num_tags = vars_extr.m_tags.size();
    for (uint32_t i = 0; i < num_tags; ++i) 
    {
      char tmp[MAX_TAG_TABLE_VARNAME];
      const uint32_t length = generate_bittable_name(vars_extr.m_tags[i], 
                                                     tmp, 
                                                     MAX_TAG_TABLE_VARNAME);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_TAG_TABLE_VARNAME);

      fprintf(fd,
              "%s = database->get_tagged_entities(\"%s\");\n",
              tmp,
              vars_extr.m_tags[i]);
    }
  }

  // INITIALIZING SYSTEM WRAPPERS
  for(uint32_t i = 0; i < stmts.size(); ++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[MAX_TYPE_NAME];
    const uint32_t system_length = fcc_type_name(match->p_system->m_system_type, 
                                                 system_name, 
                                                 MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(system_length, MAX_TYPE_NAME);

    char wrapper_name[MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 match->p_system->m_id,
                                 wrapper_name, 
                                 MAX_SYSTEM_WRAPPER_VARNAME);


    fprintf(fd,
            "%s = new %s(",
            wrapper_name,
            system_name);

    const DynArray<fcc_expr_t>& ctor_params = match->p_system->m_ctor_params;
    if(ctor_params.size() > 0) 
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

      for(size_t i = 1; i < ctor_params.size(); ++i)
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
    const uint32_t num_decls = vars_extr.m_component_decls.size();
    for (uint32_t i = 0; i < num_decls; ++i) 
    {
      generate_reflection_code(fd, vars_extr.m_component_decls[i]);
    }
  }


  fprintf(fd,"}\n");

  /// GENERATING __furious_frame CODE
  {
    fprintf(fd,"\n\n\n");
    fprintf(fd,"void __furious_frame(float delta, Database* database, void* user_data)\n{\n");

    fprintf(fd, "database->lock();\n");
    DynArray<uint32_t> seq = get_valid_exec_sequence(exec_plan);
    const uint32_t num_in_sequence = seq.size();
    for (uint32_t j = 0; j < num_in_sequence; ++j) 
    {
      fprintf(fd, "__task_%d(delta, database, user_data, 1, 0, 1);\n", seq[j]);
    }
    fprintf(fd, "database->release();\n");
    fprintf(fd, "}\n");
  }

  // GENERATING TASKS CODE BASED ON EXECUTION PLAN ROOTS
  p_registry = new CodeGenRegistry();
  num_nodes = post_exec_plan->m_subplans.size();
  for(uint32_t i = 0; i < num_nodes; ++i)
  {
    const FccOperator* root = exec_plan->m_subplans[i].p_root;
    ExecPlanPrinter printer(true);
    root->accept(&printer);
    fprintf(fd,"%s", printer.m_str_builder.p_buffer);
    fprintf(fd,"void __pf_task_%d(float delta,\n\
    Database* database,\n\
            void* user_data,\n\
            uint32_t chunk_size,\n\
            uint32_t offset,\n\
            uint32_t stride)\n", i);
    fprintf(fd,"{\n");

    fprintf(fd, "Context context(delta,database,user_data);\n");
    produce(fd,root);
    //fprintf(fd,"database->remove_temp_tables_no_lock();\n");
    fprintf(fd,"}\n");
  }
  delete p_registry;

  /// GENERATING __furious_post_frame CODE
  {
    fprintf(fd,"\n\n\n");
    fprintf(fd,"void __furious_post_frame(float delta, Database* database, void* user_data)\n{\n");

    fprintf(fd, "database->lock();\n");

    DynArray<uint32_t> seq = get_valid_exec_sequence(post_exec_plan);
    const uint32_t num_in_sequence = seq.size();
    for (uint32_t j = 0; j < num_in_sequence; ++j) 
    {
      fprintf(fd, "__pf_task_%d(delta, database, user_data, 1, 0, 1);\n", seq[j]);
    }

    fprintf(fd, "database->release();\n");
    fprintf(fd, "}\n");
  }

  // GENERATING __furious_release CODE
  fprintf(fd, "// Variable releases \n");
  fprintf(fd, "void __furious_release()\n{\n");

  for(uint32_t i = 0; i < stmts.size(); ++i)
  {
    const fcc_stmt_t* match = stmts[i];
    char system_name[MAX_TYPE_NAME];
    const uint32_t system_length = fcc_type_name(match->p_system->m_system_type, 
                                                 system_name, 
                                                 MAX_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(system_length, MAX_TYPE_NAME);

    char wrapper_name[MAX_SYSTEM_WRAPPER_VARNAME];
    const uint32_t wrapper_length = generate_system_wrapper_name(system_name, 
                                                                 match->p_system->m_id,
                                                                 wrapper_name, 
                                                                 MAX_SYSTEM_WRAPPER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(wrapper_length, MAX_SYSTEM_WRAPPER_VARNAME);


    fprintf(fd, "delete %s;\n", wrapper_name);
  }
  fprintf(fd, "}\n");
  fprintf(fd, "}\n");
  fclose(fd);
  delete [] code_buffer;
  return 0;
}

void
generate_reflection_code(FILE* fp, const QualType& decl)
{

}

} /* furious */ 
