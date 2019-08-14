


#include "driver.h"
#include "fcc_context.h"
#include "frontend/exec_plan.h"
#include "frontend/exec_plan_printer.h"
#include "drivers/clang/clang_parsing.h"
#include "drivers/clang/clang_tools.h"
#include "backend/codegen.h"

#include "stdlib.h"

#include <stdio.h>
#include <vector>

namespace furious 
{

fcc_context_t* p_fcc_context = nullptr;

fcc_entity_match_t* 
fcc_entity_match_create()
{
  fcc_entity_match_t* e_match = new fcc_entity_match_t(); 
  e_match->m_from_expand = false;
  return e_match;
}

void
fcc_entity_match_destroy(fcc_entity_match_t* fcc_entity_match)
{
  uint32_t num_tags = fcc_entity_match->m_has_tags.size();
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] fcc_entity_match->m_has_tags[i];
    
  }

  num_tags = fcc_entity_match->m_has_not_tags.size();
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] fcc_entity_match->m_has_not_tags[i];
  }

  delete fcc_entity_match;
}

fcc_stmt_t*
fcc_stmt_create()
{
  fcc_stmt_t* fcc_match = new fcc_stmt_t();
  fcc_match->m_operation_type  = fcc_operation_type_t::E_UNKNOWN;  
  fcc_match->p_system          = nullptr;                      
  fcc_match->m_priority        = 1;                            
  fcc_match->m_place           = fcc_match_place_t::E_FRAME;   
  return fcc_match;
}

void
fcc_stmt_destroy(fcc_stmt_t* fcc_match)
{
  const uint32_t num_entity_matches = fcc_match->p_entity_matches.size();
  for(uint32_t i = 0; i < num_entity_matches; ++i)
  {
    fcc_entity_match_destroy(fcc_match->p_entity_matches[i]);
  }

  if(fcc_match->p_system != nullptr)
  {
    delete fcc_match->p_system;
  }
  delete fcc_match;
}

void 
handle_parsing_error(fcc_parsing_error_type_t type,
                     const char* filename, 
                     int line,
                     int column,
                     const char* message
                    )
{

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  switch(type) 
  {
    case fcc_parsing_error_type_t::E_UNKNOWN_ERROR:
      str_builder_append(&str_builder, "Unknown error");
      break;
    case fcc_parsing_error_type_t::E_UNKNOWN_FURIOUS_OPERATION:
      str_builder_append(&str_builder, "Unknown furious operation"); 
      break;
    case fcc_parsing_error_type_t::E_INCOMPLETE_FURIOUS_STATEMENT:
      str_builder_append(&str_builder, "Incomplete furious statement ");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT:
      str_builder_append(&str_builder, "Non furious staetment");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_VAR_DECLARATIONS:
      str_builder_append(&str_builder, "Non allowed var declaration");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_TYPE_MODIFIER:
      str_builder_append(&str_builder, "Unsupported type modifier");
      break;
    case fcc_parsing_error_type_t::E_EXPECTED_STRING_LITERAL:
      str_builder_append(&str_builder, "Expected String Literal error");
      break;
    case fcc_parsing_error_type_t::E_NO_ERROR:
      str_builder_append(&str_builder, "No Error. This should never be reported");
      break;
  }

  str_builder_append(&str_builder, 
                        " found in %s:%d:%d. %s\n", 
                        filename, 
                        line, 
                        column, 
                        message);
  llvm::errs() << str_builder.p_buffer;
  str_builder_release(&str_builder);
  abort();
}

void 
handle_compilation_error(fcc_compilation_error_type_t type,
                         const char* err_msg)
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  switch(type) 
  {
    case fcc_compilation_error_type_t::E_UNKNOWN_ERROR:
      str_builder_append(&str_builder, "Unknown error: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_CYCLIC_DEPENDENCY_GRAPH:
      str_builder_append(&str_builder, "Cyclic dependency graph %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE:
      str_builder_append(&str_builder, "Invalid Column Type: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND:
      str_builder_append(&str_builder, "Invalid access mode: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_SYSTEM_INVALID_NUMBER_COMPONENTS:
      str_builder_append(&str_builder, "System invalid number of components: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_NO_ERROR:
      str_builder_append(&str_builder, "No Error. This should never be reported");
      break;
  }
  llvm::errs() << str_builder.p_buffer;
  str_builder_release(&str_builder);
  abort();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fcc_context_create() 
{
  p_fcc_context = new fcc_context_t();
  p_fcc_context->p_pecallback = nullptr;
  p_fcc_context->p_cecallback = nullptr;
  fcc_context_set_parsing_error_callback(handle_parsing_error);
  fcc_context_set_compilation_error_callback(handle_compilation_error);
}

void 
fcc_context_release()
{
  const uint32_t num_matches = p_fcc_context->p_stmts.size();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
      fcc_stmt_destroy(p_fcc_context->p_stmts[i]);
  }
  delete p_fcc_context;
}

//fcc_stmt_t*
//fcc_context_create_match(ASTContext* ast_context,
//                         Expr* expr)
//{
//  fcc_stmt_t* match = fcc_match_create();
//  match->p_ast_context = ast_context;
//  match->p_expr = expr;
//  p_fcc_context->p_stmts.append(match); 
//  return match;
//}

void 
fcc_context_set_parsing_error_callback(FCC_PARSING_ERROR_CALLBACK callback)
{
  p_fcc_context->p_pecallback = callback;
}

void 
fcc_context_set_compilation_error_callback(FCC_COMP_ERROR_CALLBACK callback)
{
  p_fcc_context->p_cecallback = callback;
}

void 
fcc_context_report_parsing_error(fcc_parsing_error_type_t error_type,
                                 const char* filename,
                                 int32_t line,
                                 int32_t column,
                                 const char* message)
{
  if(p_fcc_context->p_pecallback != nullptr) 
  {
    p_fcc_context->p_pecallback(error_type,
                                filename,
                                line,
                                column,
                                message);
  }
  abort();
}

void 
fcc_context_report_compilation_error(fcc_compilation_error_type_t error_type,
                                     const char* err_msg)
{
  if(p_fcc_context->p_cecallback != nullptr) 
  {
    p_fcc_context->p_cecallback(error_type,
                                err_msg);
  }
  abort();
}

int 
fcc_run(int argc, 
        const char** argv)
{

  fcc_context_create();

  const char* output_file = "furious_generated.cpp"; 
  const char* include_file = "furious/furious.h"; 

  for (int32_t i = 0; i < argc; ++i) 
  {
    if(strcmp("-o", argv[i]) == 0)
    {
      if(i+1 < argc)
      {
        output_file = argv[i+1];
      }
    }

    if(strcmp("-i", argv[i]) == 0)
    {
      if(i+1 < argc)
      {
        include_file = argv[i+1];
      }
    }
    
  }

  fcc_driver_init();

  // Parsing scripts
  int32_t result = fcc_parse_scripts(argc, 
                                     argv, 
                                     p_fcc_context);

  // Build execution plan
  DynArray<const fcc_stmt_t*> frame_matches;
  uint32_t num_matches = p_fcc_context->p_stmts.size();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
    const fcc_stmt_t* next_match = p_fcc_context->p_stmts[i];
    if(next_match->m_place == fcc_match_place_t::E_FRAME)
    {
      frame_matches.append(next_match);
    }
  }

  FccExecPlan* exec_plan;
  fcc_compilation_error_type_t err = create_execplan(frame_matches.buffer(), 
                                                frame_matches.size(), 
                                                &exec_plan);
  if(err != fcc_compilation_error_type_t::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  // Build post execution plan
  DynArray<const fcc_stmt_t*> post_matches;
  num_matches = p_fcc_context->p_stmts.size();
  for(uint32_t i = 0; i < num_matches; ++i)
  {
    const fcc_stmt_t* next_match = p_fcc_context->p_stmts[i];
    if(next_match->m_place == fcc_match_place_t::E_POST_FRAME)
    {
      post_matches.append(next_match);
    }
  }

  FccExecPlan* post_exec_plan;
  err = create_execplan(post_matches.buffer(), 
                        post_matches.size(), 
                        &post_exec_plan);

  if(err != fcc_compilation_error_type_t::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  llvm::errs() << "Printing Frame execplan" << "\n";
  {
    fcc_subplan_printer_t printer;
    fcc_subplan_printer_init(&printer);
    DynArray<uint32_t> seq = get_valid_exec_sequence(exec_plan);
    const uint32_t num_in_seq = seq.size();
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      fcc_subplan_printer_print(&printer, 
                                &exec_plan->m_subplans[seq[j]]);
    }
    llvm::errs() << printer.m_str_builder.p_buffer << "\n";
    fcc_subplan_printer_release(&printer);
  }

  {
    fcc_subplan_printer_t post_printer;
    fcc_subplan_printer_init(&post_printer);
    llvm::errs() << "Printing PostFrame execplan" << "\n"; 
    DynArray<uint32_t> seq = get_valid_exec_sequence(post_exec_plan);
    const uint32_t num_in_seq = seq.size();
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      fcc_subplan_printer_print(&post_printer, 
                                &post_exec_plan->m_subplans[seq[j]]);
    }
    llvm::errs() << post_printer.m_str_builder.p_buffer << "\n";
    fcc_subplan_printer_release(&post_printer);
  }

  result = fcc_generate_code(exec_plan,
                             post_exec_plan,
                             output_file,
                             include_file);
  
  destroy_execplan(exec_plan);
  destroy_execplan(post_exec_plan);
  fcc_driver_release();
  fcc_context_release();
  return result;
}


static void 
report_parsing_error_helper(fcc_expr_t expr,
                            fcc_parsing_error_type_t error_type,
                            const char* message)
{
  uint32_t line, column;
  char filename[2048];
  uint32_t length = fcc_expr_code_location(expr, 
                                           filename, 
                                           2048, 
                                           &line, 
                                           &column);
  FURIOUS_CHECK_STR_LENGTH(length, 2048);
  fcc_context_report_parsing_error(error_type,
                                   filename,
                                   line,
                                   column,
                                   message);
}


void
fcc_validate(const fcc_stmt_t* match)
{
  if(match->m_operation_type == fcc_operation_type_t::E_UNKNOWN)
  {
    report_parsing_error_helper(match->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "");
  }

  const DynArray<fcc_entity_match_t*>& e_matches = match->p_entity_matches;
  uint32_t num_matches = e_matches.size();
  if( num_matches == 0 )
  {

    report_parsing_error_helper(match->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "At least one match must exist");
  }

  if( num_matches > 2)
  {

    report_parsing_error_helper(match->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "More than 2 expands found. The maximum number of expands per statement is 1");
  }

  DynArray<bool> allow_writes;
  DynArray<bool> allow_globals;

  fcc_entity_match_t* e_match = e_matches[num_matches - 1];
  for(uint32_t i = 0; i < e_match->m_component_types.size(); ++i)
  {
    allow_writes.append(true);
    allow_globals.append(true);
  }

  for(int32_t i = num_matches - 2; i >= 0; --i)
  {
    e_match = e_matches[i];
    for(uint32_t j = 0; j < e_match->m_component_types.size(); ++j)
    {
      allow_writes.append(false);
      allow_globals.append(false);
    }
  }

  const DynArray<fcc_component_match_t>& matches = match->p_system->m_component_types;
  if(allow_writes.size() != matches.size())
  {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder,"Match: %u, System: %u", allow_writes.size(), matches.size());
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_SYSTEM_INVALID_NUMBER_COMPONENTS,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
  }

  bool all_globals = true;
  for(uint32_t i = 0; i < matches.size(); ++i)
  {
    const fcc_component_match_t* match_type = &matches[i];
    if(allow_writes[i] == false &&
       !match_type->m_is_read_only)
    {
      char ctype[MAX_TYPE_NAME];
      uint32_t length = fcc_type_name(match_type->m_type,
                                      ctype,
                                      MAX_TYPE_NAME);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                            "\"%s\" access mode after expand\
                            must be read-only", 
                            ctype);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    if(allow_globals[i] == false &&
       match_type->m_is_global)
    {
      char ctype[MAX_TYPE_NAME];
      const uint32_t length = fcc_type_name(match_type->m_type,
                                            ctype,
                                            MAX_TYPE_NAME);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder,
                            "\"%s\" scope modifier after\
                            expand cannot be global", 
                            ctype);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                           str_builder.p_buffer);
      str_builder_release(&str_builder);
    }
    all_globals &= match_type->m_is_global;
  }

  if(!all_globals)
  {
    for(uint32_t i = 0; i < matches.size(); ++i)
    {
      const fcc_component_match_t* match_type = &matches[i];
      if(!match_type->m_is_read_only && match_type->m_is_global)
      {
        char ctype[MAX_TYPE_NAME];
        const uint32_t length = fcc_type_name(match_type->m_type,
                                              ctype,
                                              MAX_TYPE_NAME);

        FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

        str_builder_t str_builder;
        str_builder_init(&str_builder);
        str_builder_append(&str_builder, 
                              "\"%s\" read-write access mode on globals is \
                              only allowed \"global-only\" systems", 
                              ctype);
        fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                             str_builder.p_buffer);
        str_builder_release(&str_builder);
      }
    }
  }

}

} /* furious */ 
