

#include "../frontend/exec_plan_printer.h"
#include "../frontend/execution_plan.h"
#include "../clang_tools.h"
#include "codegen.h"
#include "codegen_tools.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "../structs.h"


#include <sstream>
#include <fstream>

namespace furious {


/**
 * @brief Visitor used to extract the dependencies of an execution plan
 */
class DependenciesExtr : public FccExecPlanVisitor 
{
public:

  std::set<std::string> m_include_files;
  std::set<Decl*>       m_declarations;

  void
  extract_dependencies(const std::vector<Dependency>& deps)
  {
      for(const Dependency& dep : deps) 
      {
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
    for(const FccSystemInfo& info : foreach->m_systems)
    {
      extract_dependencies(get_dependencies(info.m_system_type));
    }
    foreach->p_child->accept(this);
  }

  virtual void 
  visit(const Scan* scan) 
  {
    extract_dependencies(get_dependencies(scan->m_component));
  }

  virtual void
  visit(const Join* join) 
  {
    join->p_left->accept(this);
    join->p_right->accept(this);
  }

  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    tag_filter->p_child->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    extract_dependencies(get_dependencies(component_filter->m_component_type));
    component_filter->p_child->accept(this);
  }

  virtual void
  visit(const PredicateFilter* predicate_filter) 
  {
    extract_dependencies(get_dependencies(predicate_filter->p_func_decl));
    predicate_filter->p_child->accept(this);
  }
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


class VarsExtr : public FccExecPlanVisitor 
{
public:
  std::set<std::string> m_components;
  std::set<std::string> m_tags;

  virtual void 
  visit(const Foreach* foreach)
  {
    foreach->p_child->accept(this);
  }

  virtual void 
  visit(const Scan* scan) 
  {
    m_components.insert(get_type_name(scan->m_component));
  }

  virtual void
  visit(const Join* join) 
  {
    join->p_left->accept(this);
    join->p_right->accept(this);
  }

  virtual void 
  visit(const TagFilter* tag_filter) 
  {
    m_tags.insert(tag_filter->m_tag);
    tag_filter->p_child->accept(this);
  }

  virtual void
  visit(const ComponentFilter* component_filter) 
  {
    m_components.insert(get_type_name(component_filter->m_component_type));
    component_filter->p_child->accept(this);
  }

  virtual void
  visit(const PredicateFilter* predicate_filter) 
  {
    predicate_filter->p_child->accept(this);
  }
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void 
generate_code(const FccExecPlan* exec_plan,
              const std::string& filename)
{
  std::stringstream output_ss;
  output_ss << "\n\n\n";
  // add basic includes
  output_ss << "#include <furious.h> \n";
  /// Find dependencies
  DependenciesExtr deps_visitor;
  deps_visitor.traverse(exec_plan);
  for(const std::string& incl : deps_visitor.m_include_files)
  {
    output_ss << "#include \""<< incl <<"\"\n";
  }

  output_ss << "\n\n\n";

  for(const Decl* decl : deps_visitor.m_declarations)
  {
    const SourceManager& sm = decl->getASTContext().getSourceManager();
    SourceLocation start = decl->getLocStart();
    SourceLocation end = decl->getLocEnd();
    std::string code = get_code(sm,
                                start,
                                end);
    output_ss << code << ";\n\n";
  }


  output_ss << "namespace furious \n{\n";
  /// Declare variables (e.g. tableviews, tag sets, etc.)
  output_ss << "\n\n\n";
  output_ss << "// Variable declarations \n";
  VarsExtr vars_extr;
  vars_extr.traverse(exec_plan);
  for(const std::string& table : vars_extr.m_components)
  {
    // TableViews
    std::string base_name = table;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    std::string table_varname = base_name+"_table";
    output_ss << "TableView<"<<table<<"> " <<table_varname<< ";\n";
  }

  for(const std::string& tag : vars_extr.m_tags)
  {
    // TagSets
    output_ss << "BitTable* tagged_"<<tag<< ";\n";
  }

  for(const FccExecInfo& info : exec_plan->p_context->m_operations)
  {
    std::string system_name = get_type_name(info.m_system.m_system_type);
    std::string base_name = system_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    output_ss << "SystemWrapper<"<< system_name <<">* "<< base_name << "_" << info.m_system.m_id << ";\n";
  }

  /// Initialize variables
  output_ss << "\n\n\n";
  output_ss << "// Variable initializations \n";
  output_ss << "void __furious_init()\n{\n";
  for(const std::string& table : vars_extr.m_components)
  {
    std::string base_name = table;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    std::string table_varname = base_name+"_table";
    output_ss << table_varname<< " = database->find_table<"<<table<<">();\n";
  }

  for(const std::string& tag : vars_extr.m_tags)
  {
    output_ss << "tagged_"<< tag << " = database->get_tagged_entities(\""<<tag<<"\");\n";
  }

  for(const FccExecInfo& info : exec_plan->p_context->m_operations)
  {
    std::string system_name = get_type_name(info.m_system.m_system_type);
    std::string base_name = system_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    output_ss << base_name << "_" << info.m_system.m_id << " = create_system<"<<system_name<<">(";

    size_t num_params = info.m_system.m_ctor_params.size();
    if( num_params > 0) 
    {
      const Expr* param = info.m_system.m_ctor_params[0];
      const SourceManager& sm = info.p_ast_context->getSourceManager();
      SourceLocation start = param->getLocStart();
      SourceLocation end = param->getLocEnd();
      std::string code = get_code(sm, 
                                  start,
                                  end);
        output_ss << code;
      for(size_t i = 1; i < num_params; ++i)
      {
        const Expr* param = info.m_system.m_ctor_params[i];
        SourceLocation start = param->getLocStart();
        SourceLocation end = param->getLocEnd();
        std::string code = get_code(sm, 
                                    start,
                                    end);
        output_ss << "," << code;
      }
    }
    output_ss << ");\n";
  }
  output_ss << "}\n";

  /// Generate execution code
  output_ss << "\n\n\n";
  output_ss << "void __furious_frame(float_t delta)\n{\n";

  output_ss << "Context context{delta,database};\n";

  for(const FccOperator* root : exec_plan->m_roots)
  {
    ExecPlanPrinter printer{true};
    root->accept(&printer);
    output_ss << printer.m_string_builder.str();
    output_ss << "{\n";
    produce(output_ss,root);
    output_ss << "}\n";
  }
  output_ss << "}\n";

  output_ss << "// Variable releases \n";
  output_ss << "void __furious_release()\n{\n";

  for(const FccExecInfo& info : exec_plan->p_context->m_operations)
  {
    std::string system_name = get_type_name(info.m_system.m_system_type);
    std::string base_name = system_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    output_ss << "destroy_system("<< base_name << "_" << info.m_system.m_id << ");\n";
  }
  output_ss << "}\n";
  output_ss << "}\n";
  std::ofstream output_file(filename);
  output_file << output_ss.str();
  output_file.close();
}
  
} /* furious */ 
