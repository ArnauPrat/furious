

#include "../clang_tools.h"
#include "../common/dyn_array.h"
#include "../fcc_context.h"
#include "../frontend/exec_plan_printer.h"
#include "../frontend/execution_plan.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "consume_visitor.h"
#include "produce_visitor.h"

#include <stdio.h>
#include <stdlib.h>

namespace furious 
{

CodeGenRegistry* p_registry = nullptr;;

/**
 * @brief Visitor used to extract the dependencies of an execution plan, which
 * include structures and headers including dependent structures
 */
class DependenciesExtr : public FccExecPlanVisitor 
{
public:

  std::set<std::string>   m_include_files;
  std::set<const Decl*>   m_declarations;

  void
  extract_dependencies(const DynArray<Dependency>& deps)
  {
      for(uint32_t i = 0; i < deps.size(); ++i) 
      {
        const Dependency& dep = deps[i];
        if(dep.p_decl != nullptr) 
        {
          const Decl* decl = dep.p_decl;
          if(isa<TagDecl>(decl) || 
            (isa<FunctionDecl>(decl) && !cast<FunctionDecl>(decl)->isCXXClassMember()))
          {
            m_declarations.insert(dep.p_decl);
          }
        } else 
        {
          m_include_files.insert(dep.m_include_file);
        }
      }
  }

  virtual void 
  visit(const Foreach* foreach)
  {
    uint32_t size = foreach->p_systems.size();
    for(uint32_t i = 0; i < size; ++i)
    {
      const FccSystem* system = foreach->p_systems[i];
      extract_dependencies(get_dependencies(system->m_system_type));
    }
    foreach->p_child.get()->accept(this);
  }

  virtual void 
  visit(const Scan* scan) 
  {
    if(scan->m_columns[0].m_type == FccColumnType::E_COMPONENT)
    {
      extract_dependencies(get_dependencies(scan->m_columns[0].m_q_type));
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
      extract_dependencies(get_dependencies(fetch->m_columns[0].m_q_type));
  }

  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    tag_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    extract_dependencies(get_dependencies(component_filter->m_filter_type));
    component_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const PredicateFilter* predicate_filter) 
  {
    extract_dependencies(get_dependencies(predicate_filter->p_func_decl));
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
class VarsExtr : public FccExecPlanVisitor 
{
public:
  std::set<std::string> m_components;
  std::set<std::string> m_tags;
  std::set<std::string> m_references;

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
      m_components.insert(get_type_name(scan->m_columns[0].m_q_type));
    }
    else
    {
      m_references.insert(scan->m_columns[0].m_ref_name);
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
    m_components.insert(get_type_name(fetch->m_columns[0].m_q_type));
  }


  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    m_tags.insert(tag_filter->m_tag);
    tag_filter->p_child.get()->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    m_components.insert(get_type_name(component_filter->m_filter_type));
    component_filter->p_child.get()->accept(this);
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

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename,
              const std::string& include_file)
{
  FILE* fd = fopen(filename.c_str(), "w");
  fprintf(fd, "\n\n\n");
  // ADDING FURIOUS INCLUDE 
  fprintf(fd, "#include <%s> \n", include_file.c_str());

  // LOOKING FOR DEPENDENCIES 
  DependenciesExtr deps_visitor;
  deps_visitor.traverse(exec_plan);

  // ADDING REQUIRED INCLUDES
  for(const std::string& incl : deps_visitor.m_include_files)
  {
    fprintf(fd, "#include \"%s\"\n", incl.c_str());
  }

  fprintf(fd, "\n\n\n");

  // ADDING REQUIRED "USING NAMESPACE DIRECTIVES TODO: Add support for other
  // using clauses"
  for(uint32_t i = 0; i < exec_plan->p_context->p_using_decls.size(); ++i)
  {
    const UsingDirectiveDecl* decl = exec_plan->p_context->p_using_decls[i];
    const SourceManager& sm = decl->getASTContext().getSourceManager();
    std::string code = get_code(sm, decl->getSourceRange());
    fprintf(fd,"%s;\n",code.c_str());
  }

  // ADDING DECLARATIONS FOUND IN FURIOUS SCRIPTS
  for(const Decl* decl : deps_visitor.m_declarations)
  {
    const SourceManager& sm = decl->getASTContext().getSourceManager();
    std::string code = get_code(sm,
                                decl->getSourceRange());
    fprintf(fd,"%s;\n\n", code.c_str());
  }

  // STARTING CODE GENERATION
  fprintf(fd,"namespace furious \n{\n");

  /// DECLARE VARIABLES (e.g. TABLEVIEWS, BITTABLES, etc.)
  fprintf(fd, "\n\n\n");
  fprintf(fd,"// Variable declarations \n");
  VarsExtr vars_extr;
  vars_extr.traverse(exec_plan);

  // TABLEVIEWS
  for(const std::string& table : vars_extr.m_components)
  {
    std::string table_varname = generate_table_name(table);
    fprintf(fd, "TableView<%s> %s;\n", table.c_str(), table_varname.c_str());
  }

  for(const std::string& ref_name : vars_extr.m_references)
  {
    std::string table_varname = generate_ref_table_name(ref_name);
    fprintf(fd, "TableView<entity_id_t> %s;\n", table_varname.c_str());
  }

  // BITTABLES
  for(const std::string& tag : vars_extr.m_tags)
  {
    std::string table_varname = generate_bittable_name(tag);
    fprintf(fd, "BitTable* %s;\n", table_varname.c_str());
  }

  // SYSTEMWRAPPERS
  for(uint32_t i = 0; i < exec_plan->p_context->p_matches.size();++i)
  {
    const FccMatch* match = exec_plan->p_context->p_matches[i];
    std::string system_name = get_type_name(match->m_system.m_system_type);
    std::string wrapper_name = generate_system_wrapper_name(system_name, 
                                                            match->m_system.m_id);
    fprintf(fd, "%s* %s;", system_name.c_str(), wrapper_name.c_str());
  }

  /// GENERATING __furious__init  
  fprintf(fd, "\n\n\n");
  fprintf(fd, "// Variable initializations \n");
  fprintf(fd, "void __furious_init(Database* database)\n{\n");

  // INITIALIZING TABLEVIEWS 
  for(const std::string& table : vars_extr.m_components)
  {
    std::string table_varname = generate_table_name(table);
    fprintf(fd,
            "%s  = FURIOUS_FIND_OR_CREATE_TABLE(database, %s);\n",
            table_varname.c_str(),
            table.c_str());
  }

  for(const std::string& ref_name : vars_extr.m_references)
  {
    std::string table_varname = generate_ref_table_name(ref_name);
    fprintf(fd,
            "%s  = database->get_references(\"%s\");\n",
            table_varname.c_str(),
            ref_name.c_str());
  }

  // INITIALIZING BITTABLES
  for(const std::string& tag : vars_extr.m_tags)
  {
    std::string table_name = generate_bittable_name(tag);
    fprintf(fd,
            "%s = database->get_tagged_entities(\"%s\");\n",
            table_name.c_str(),
            tag.c_str());
  }

  // INITIALIZING SYSTEM WRAPPERS
  for(uint32_t i = 0; i < exec_plan->p_context->p_matches.size(); ++i)
  {
    const FccMatch* match = exec_plan->p_context->p_matches[i];
    std::string system_name = get_type_name(match->m_system.m_system_type);
    std::string wrapper_name = generate_system_wrapper_name(system_name, 
                                                            match->m_system.m_id);
    fprintf(fd,
            "%s = new %s(",
            wrapper_name.c_str(),
            system_name.c_str());

    size_t num_params = match->m_system.m_ctor_params.size();
    if( num_params > 0) 
    {
      const Expr* param = match->m_system.m_ctor_params[0];
      const SourceManager& sm = match->p_ast_context->getSourceManager();
      std::string code = get_code(sm,param->getSourceRange());
      fprintf(fd,"%s", code.c_str());
      for(size_t i = 1; i < num_params; ++i)
      {
        const Expr* param = match->m_system.m_ctor_params[i];
        std::string code = get_code(sm,param->getSourceRange());
        fprintf(fd,",%s",code.c_str());
      }
    }
    fprintf(fd,");\n");
  }
  fprintf(fd,"}\n");

  /// GENERATING __furious_frame CODE
  fprintf(fd,"\n\n\n");
  fprintf(fd,"void __furious_frame(float delta, Database* database)\n{\n");

  fprintf(fd, "database->lock();\n");
  fprintf(fd, "Context context(delta,database);\n");

  // GENERATING CODE BASED ON EXECUTION PLAN ROOTS
  p_registry = new CodeGenRegistry();
  for(uint32_t i = 0; i < exec_plan->p_roots.size(); ++i)
  {
    const FccOperator* root = exec_plan->p_roots[i];
    ExecPlanPrinter printer(true);
    root->accept(&printer);
    fprintf(fd,"%s", printer.m_string_builder.p_buffer);
    fprintf(fd,"{\n");
    produce(fd,root);
    //fprintf(fd,"database->remove_temp_tables_no_lock();\n");
    fprintf(fd,"}\n");
  }
  delete p_registry;

  fprintf(fd, "database->release();\n");
  fprintf(fd, "}\n");

  // GENERATING __furious_release CODE
  fprintf(fd, "// Variable releases \n");
  fprintf(fd, "void __furious_release()\n{\n");

  for(uint32_t i = 0; i < exec_plan->p_context->p_matches.size(); ++i)
  {
    const FccMatch* match = exec_plan->p_context->p_matches[i];
    std::string system_name = get_type_name(match->m_system.m_system_type);
    std::string wrapper_name = generate_system_wrapper_name(system_name, 
                                                            match->m_system.m_id);
    fprintf(fd, "delete %s;\n", wrapper_name.c_str());
  }
  fprintf(fd, "}\n");
  fprintf(fd, "}\n");
  fclose(fd);
}
  
} /* furious */ 
