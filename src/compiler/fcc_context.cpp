
#include "backend/codegen.h"
#include "driver.h"
#include "fcc_context.h"
#include "frontend/exec_plan.h"
#include "frontend/exec_plan_printer.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

fcc_context_t* p_fcc_context = nullptr;

void
fcc_system_add_ctor_param(fcc_system_t* sys, fcc_expr_t ctorp)
{
  FDB_PERMA_ASSERT(sys->m_nctor_params < FCC_MAX_CTOR_PARAMS && 
                       "Maximum number of ctor parameters in system exceeded");
  sys->m_ctor_params[sys->m_nctor_params++] = ctorp;
}

void
fcc_system_add_cmatch(fcc_system_t* sys, fcc_component_match_t cmatch)
{
  FDB_PERMA_ASSERT(sys->m_ncmatches < FCC_MAX_SYSTEM_COMPONENTS && 
                       "Maximum number of components in system exceeded");
  sys->m_cmatches[sys->m_ncmatches++] = cmatch;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

fcc_entity_match_t
fcc_entity_match_init()
{
  fcc_entity_match_t ematch;
  ematch = {};
  return ematch;
}

void
fcc_entity_match_release(fcc_entity_match_t* fcc_entity_match)
{
  uint32_t num_tags = fcc_entity_match->m_nhas_tags;
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] fcc_entity_match->m_has_tags[i];
  }

  num_tags = fcc_entity_match->m_nhas_not_tags;
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] fcc_entity_match->m_has_not_tags[i];
  }
}

void
fcc_entity_match_add_cmatch(fcc_entity_match_t* ematch, 
                            fcc_component_match_t cmatch)
{
  FDB_PERMA_ASSERT(ematch->m_ncmatches < FCC_MAX_SYSTEM_COMPONENTS &&
                       "Maximum number of components in match exceeded");
  ematch->m_cmatches[ematch->m_ncmatches++] = cmatch;
}

void
fcc_entity_match_add_has_comp(fcc_entity_match_t* ematch, 
                              fcc_type_t comp)
{
  FDB_PERMA_ASSERT(ematch->m_nhas_components < FCC_MAX_HAS_COMPONENTS &&
                       "Maximum number of has components in match exceeded");
  ematch->m_has_components[ematch->m_nhas_components++] = comp;
}

void
fcc_entity_match_add_has_not_comp(fcc_entity_match_t* ematch, 
                                  fcc_type_t comp)
{
  FDB_PERMA_ASSERT(ematch->m_nhas_not_components < FCC_MAX_HAS_NOT_COMPONENTS &&
                       "Maximum number of has not components in match exceeded");
  ematch->m_has_not_components[ematch->m_nhas_not_components++] = comp;
}

void
fcc_entity_match_add_has_tag(fcc_entity_match_t* ematch, 
                             const char* tag)
{
  FDB_PERMA_ASSERT(ematch->m_nhas_tags < FCC_MAX_HAS_TAGS &&
                       "Maximum number of has tags in match exceeded");
  char* buffer = new char[FCC_MAX_TAG_NAME];
  FDB_COPY_AND_CHECK_STR(buffer, tag, FCC_MAX_TAG_NAME);
  ematch->m_has_tags[ematch->m_nhas_tags++] = buffer;
}

void
fcc_entity_match_add_has_not_tag(fcc_entity_match_t* ematch, 
                             const char* tag)
{
  FDB_PERMA_ASSERT(ematch->m_nhas_not_tags < FCC_MAX_HAS_NOT_TAGS &&
                       "Maximum number of has not tags in match exceeded");
  char* buffer = new char[FCC_MAX_TAG_NAME];
  FDB_COPY_AND_CHECK_STR(buffer, tag, FCC_MAX_TAG_NAME);
  ematch->m_has_not_tags[ematch->m_nhas_not_tags++] = buffer;
}

void
fcc_entity_match_add_filter(fcc_entity_match_t* ematch, 
                            fcc_decl_t fdecl)
{
  FDB_PERMA_ASSERT(ematch->m_nfuncs < FCC_MAX_FILTER_FUNC &&
                       "Maximum number of filter funcs in match exceeded");
  ematch->m_filter_func[ematch->m_nfuncs++] = fdecl;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

fcc_stmt_t
fcc_stmt_init()
{
  fcc_stmt_t fcc_stmt = {};
  fcc_stmt.m_ematches = fcc_ematch_ptr_array_init();
  fcc_stmt.p_system = nullptr;
  fcc_stmt.m_place = fcc_stmt_place_t::E_FRAME;
  fcc_stmt.m_priority = 1;
  return fcc_stmt;
}

void
fcc_stmt_release(fcc_stmt_t* fcc_stmt)
{
  const uint32_t nmatches = fcc_stmt->m_ematches.m_count;
  fcc_entity_match_t** ematches = fcc_stmt->m_ematches.m_data;
  for(uint32_t i = 0; i < nmatches; ++i)
  {
    fcc_entity_match_release(ematches[i]);
    delete ematches[i];
  }

  if(fcc_stmt->p_system != nullptr)
  {
    delete fcc_stmt->p_system;
  }

  fcc_ematch_ptr_array_release(&fcc_stmt->m_ematches);
}

void 
handle_parsing_error(fcc_parsing_error_type_t type,
                     const char* filename, 
                     int line,
                     int column,
                     const char* message
                    )
{

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  switch(type) 
  {
    case fcc_parsing_error_type_t::E_UNKNOWN_ERROR:
      fdb_str_builder_append(&str_builder, "Unknown error");
      break;
    case fcc_parsing_error_type_t::E_UNKNOWN_FDB_OPERATION:
      fdb_str_builder_append(&str_builder, "Unknown furious operation"); 
      break;
    case fcc_parsing_error_type_t::E_INCOMPLETE_FDB_STATEMENT:
      fdb_str_builder_append(&str_builder, "Incomplete furious statement ");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT:
      fdb_str_builder_append(&str_builder, "Non furious staetment");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_VAR_DECLARATIONS:
      fdb_str_builder_append(&str_builder, "Non allowed var declaration");
      break;
    case fcc_parsing_error_type_t::E_UNSUPPORTED_TYPE_MODIFIER:
      fdb_str_builder_append(&str_builder, "Unsupported type modifier");
      break;
    case fcc_parsing_error_type_t::E_EXPECTED_STRING_LITERAL:
      fdb_str_builder_append(&str_builder, "Expected String Literal error");
      break;
    case fcc_parsing_error_type_t::E_NO_ERROR:
      fdb_str_builder_append(&str_builder, "No Error. This should never be reported");
      break;
  }

  fdb_str_builder_append(&str_builder, 
                        " found in %s:%d:%d. %s\n", 
                        filename, 
                        line, 
                        column, 
                        message);
  printf("%s\n", str_builder.p_buffer);
  fdb_str_builder_release(&str_builder);
  abort();
}

void 
handle_compilation_error(fcc_compilation_error_type_t type,
                         const char* err_msg)
{
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  switch(type) 
  {
    case fcc_compilation_error_type_t::E_UNKNOWN_ERROR:
      fdb_str_builder_append(&str_builder, "Unknown error: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_CYCLIC_DEPENDENCY_GRAPH:
      fdb_str_builder_append(&str_builder, "Cyclic dependency graph %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE:
      fdb_str_builder_append(&str_builder, "Invalid Column Type: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND:
      fdb_str_builder_append(&str_builder, "Invalid access mode: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_SYSTEM_INVALID_NUMBER_COMPONENTS:
      fdb_str_builder_append(&str_builder, "System invalid number of components: %s\n", err_msg);
      break;
    case fcc_compilation_error_type_t::E_NO_ERROR:
      fdb_str_builder_append(&str_builder, "No Error. This should never be reported");
      break;
  }
  printf("%s\n", str_builder.p_buffer);
  fdb_str_builder_release(&str_builder);
  abort();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fcc_context_init() 
{
  p_fcc_context = new fcc_context_t();
  *p_fcc_context = {};
  p_fcc_context->p_pecallback = nullptr;
  p_fcc_context->p_cecallback = nullptr;
  p_fcc_context->m_stmts = fcc_stmt_ptr_array_init();
  p_fcc_context->m_using_decls = fcc_decl_array_init();
  fcc_context_set_parsing_error_callback(handle_parsing_error);
  fcc_context_set_compilation_error_callback(handle_compilation_error);
}

void 
fcc_context_release()
{
  fcc_stmt_t** stmts = p_fcc_context->m_stmts.m_data;
  const uint32_t nstmts = p_fcc_context->m_stmts.m_count;
  for(uint32_t i = 0; i < nstmts; ++i)
  {
      fcc_stmt_release(stmts[i]);
      delete stmts[i];
  }
  fcc_stmt_ptr_array_release(&p_fcc_context->m_stmts);
  fcc_decl_array_release(&p_fcc_context->m_using_decls);
  delete p_fcc_context;
}

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
FCC_CONTEXT_REPORT_PARSING_ERROR(fcc_parsing_error_type_t error_type,
                                 const char* filename,
                                 int32_t line,
                                 int32_t column,
                                 const char* message, 
                                 ...)
{

  va_list myargs;
  va_start(myargs, message);
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, message, myargs);
  if(p_fcc_context->p_pecallback != nullptr) 
  {
    p_fcc_context->p_pecallback(error_type,
                                filename,
                                line,
                                column,
                                message);
  }
  va_end(myargs);
  fdb_str_builder_release(&str_builder);
  abort();
}

void 
FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t error_type,
                                     const char* err_msg, 
                                     ...)
{

  va_list myargs;
  va_start(myargs, err_msg);
  fdb_str_builder_t str_builder; 
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, err_msg, myargs);
  if(p_fcc_context->p_cecallback != nullptr) 
  {
    p_fcc_context->p_cecallback(error_type,
                                str_builder.p_buffer);
  }
  va_end(myargs);
  fdb_str_builder_release(&str_builder);
  abort();
}

int 
fcc_run(int argc, 
        const char** argv)
{

  fcc_context_init();

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
  fcc_stmt_t** stmts = p_fcc_context->m_stmts.m_data;
  uint32_t nstmts = p_fcc_context->m_stmts.m_count;
  const fcc_stmt_t* frame_matches[nstmts];
  uint32_t nfstmts = 0;
  for(uint32_t i = 0; i < nstmts; ++i)
  {
    const fcc_stmt_t* next_match = stmts[i];
    if(next_match->m_place == fcc_stmt_place_t::E_FRAME)
    {
      frame_matches[nfstmts++] = next_match;
    }
  }

  fcc_exec_plan_t exec_plan;
  fcc_compilation_error_type_t err = fcc_exec_plan_init(&exec_plan, 
                                                          frame_matches, 
                                                          nfstmts);
  if(err != fcc_compilation_error_type_t::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  // Build post execution plan
  const fcc_stmt_t* post_matches[nstmts];
  uint32_t npstmts = 0; 
  for(uint32_t i = 0; i < nstmts; ++i)
  {
    const fcc_stmt_t* next_match = stmts[i];
    if(next_match->m_place == fcc_stmt_place_t::E_POST_FRAME)
    {
      post_matches[npstmts++] = next_match;
    }
  }

  fcc_exec_plan_t post_exec_plan;
  err = fcc_exec_plan_init(&post_exec_plan, 
                             post_matches, 
                             npstmts);

  if(err != fcc_compilation_error_type_t::E_NO_ERROR)
  {
    handle_compilation_error(err, "");
  }

  printf("Printing Frame execplan\n");
  {
    fcc_subplan_printer_t printer;
    fcc_subplan_printer_init(&printer);
    uint32_t cseq[exec_plan.m_nnodes];
    fcc_exec_plan_get_valid_exec_sequence(&exec_plan, 
                                          cseq, 
                                          exec_plan.m_nnodes);
    const uint32_t num_in_seq = exec_plan.m_nnodes;
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      fcc_subplan_printer_print(&printer, 
                                exec_plan.m_subplans[cseq[j]]);
    }
    printf("%s\n", printer.m_str_builder.p_buffer);
    fcc_subplan_printer_release(&printer);
  }

  {
    fcc_subplan_printer_t post_printer;
    fcc_subplan_printer_init(&post_printer);
    printf("Printing PostFrame execplan"); 
    uint32_t pseq[post_exec_plan.m_nnodes];
    fcc_exec_plan_get_valid_exec_sequence(&post_exec_plan, 
                                          pseq, 
                                          post_exec_plan.m_nnodes);
    const uint32_t num_in_seq = post_exec_plan.m_nnodes;
    for(uint32_t j = 0; 
        j < num_in_seq; 
        ++j)
    {
      fcc_subplan_printer_print(&post_printer, 
                                post_exec_plan.m_subplans[pseq[j]]);
    }
    printf("%s\n", post_printer.m_str_builder.p_buffer);
    fcc_subplan_printer_release(&post_printer);
  }

  result = fcc_generate_code(&exec_plan,
                             &post_exec_plan,
                             output_file,
                             include_file);
  
  fcc_exec_plan_release(&exec_plan);
  fcc_exec_plan_release(&post_exec_plan);
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
  fcc_expr_code_location(expr, 
                         filename, 
                         2048, 
                         &line, 
                         &column);
  FCC_CONTEXT_REPORT_PARSING_ERROR(error_type,
                                   filename,
                                   line,
                                   column,
                                   message);
}


void
fcc_validate(const fcc_stmt_t* stmt)
{
  if(stmt->m_operation_type == fcc_operation_type_t::E_UNKNOWN)
  {
    report_parsing_error_helper(stmt->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "");
  }

  const uint32_t nmatches = stmt->m_ematches.m_count;
  fcc_entity_match_t** ematches = stmt->m_ematches.m_data;
  if(nmatches == 0 )
  {

    report_parsing_error_helper(stmt->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "At least one match must exist");
  }

  if( nmatches > 2)
  {

    report_parsing_error_helper(stmt->m_expr,
                                fcc_parsing_error_type_t::E_UNSUPPORTED_STATEMENT,
                                "More than 2 expands found. The maximum number of expands per statement is 1");
  }

  // count components from matches
  uint32_t ncomps = 0;
  for(uint32_t i = 0; i < nmatches; ++i)
  {
    fcc_entity_match_t* ematch = ematches[i];
    ncomps+=ematch->m_ncmatches;
  }
  bool allow_writes[ncomps];
  bool allow_globals[ncomps];

  fcc_entity_match_t* e_match = ematches[nmatches - 1];
  uint32_t ccomps = 0;
  for(uint32_t i = 0; i < e_match->m_ncmatches; ++i)
  {
    allow_writes[ccomps] = true;
    allow_globals[ccomps] = true;
    ccomps++;
  }

  for(int32_t i = nmatches - 2; i >= 0; --i)
  {
    e_match = ematches[i];
    for(uint32_t j = 0; j < e_match->m_ncmatches; ++j)
    {
      allow_writes[ccomps] = true;
      allow_globals[ccomps] = true;
      ccomps++;
    }
  }

  uint32_t ncmatches = stmt->p_system->m_ncmatches; 
  if(ncomps != ncmatches)
  {
      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_SYSTEM_INVALID_NUMBER_COMPONENTS,
                                           "Match: %u, System: %u", ncomps, ncmatches);
  }

  fcc_component_match_t* cmatches = stmt->p_system->m_cmatches;
  bool all_globals = true;
  for(uint32_t i = 0; i < ncmatches; ++i)
  {
    const fcc_component_match_t* match_type = &cmatches[i];
    if(allow_writes[i] == false &&
       !match_type->m_is_read_only)
    {
      char ctype[FCC_MAX_TYPE_NAME];
      fcc_type_name(match_type->m_type,
                    ctype,
                    FCC_MAX_TYPE_NAME);


      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                           "\"%s\" access mode after expand\
                                           must be read-only");
    }

    if(allow_globals[i] == false &&
       match_type->m_is_global)
    {
      char ctype[FCC_MAX_TYPE_NAME];
      fcc_type_name(match_type->m_type,
                    ctype,
                    FCC_MAX_TYPE_NAME);

      FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                           "\"%s\" scope modifier after\
                                           expand cannot be global", 
                                           ctype);
    }
    all_globals &= match_type->m_is_global;
  }

  if(!all_globals)
  {
    for(uint32_t i = 0; i < ncmatches; ++i)
    {
      const fcc_component_match_t* match_type = &cmatches[i];
      if(!match_type->m_is_read_only && match_type->m_is_global)
      {
        char ctype[FCC_MAX_TYPE_NAME];
        fcc_type_name(match_type->m_type,
                      ctype,
                      FCC_MAX_TYPE_NAME);

        FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t::E_INVALID_ACCESS_MODE_ON_EXPAND,
                                             "\"%s\" read-write access mode on globals is \
                                             only allowed \"global-only\" systems");
      }
    }
  }

} /* furious */ 
