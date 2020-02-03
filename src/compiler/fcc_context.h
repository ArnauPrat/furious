
#ifndef _FDB_COMPILER_STRUCTS_H_
#define _FDB_COMPILER_STRUCTS_H_

#include "../common/platform.h"

// autogen includes
#include "autogen/fcc_ematch_ptr_array.h"
#include "autogen/fcc_stmt_ptr_array.h"
#include "autogen/fcc_decl_array.h"

struct fcc_context_t;
extern fcc_context_t* p_fcc_context;

typedef void* fcc_type_t;
typedef void* fcc_expr_t;
typedef void* fcc_decl_t;

enum class fcc_operation_type_t 
{
  E_UNKNOWN = 0,
  E_FOREACH
};

enum class fcc_access_mode_t 
{
  E_READ = 0,
  E_READ_WRITE,
};

struct fcc_component_match_t 
{
  fcc_type_t  m_type;
  bool        m_is_read_only;
  bool        m_is_global;
};

/**
 * \brief 
 */
struct fcc_system_t 
{
  fcc_type_t                      m_system_type;                                // The type of the system
  fcc_expr_t                      m_ctor_params[FCC_MAX_CTOR_PARAMS];           // The expressions of the system's constructor parameters 
  uint32_t                        m_nctor_params;                               // The number of ctor parameters
  fcc_component_match_t           m_cmatches[FCC_MAX_SYSTEM_COMPONENTS];        // The types of the components of the system
  uint32_t                        m_ncmatches;                                  // Number of component types
  int32_t                         m_id;                                         // The id of the system  
};

void
fcc_system_add_ctor_param(fcc_system_t* sys, fcc_expr_t ctorp);

void
fcc_system_add_cmatch(fcc_system_t* sys, fcc_component_match_t cmatch);


/**
 * \brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct fcc_entity_match_t 
{
  fcc_component_match_t         m_cmatches[FCC_MAX_SYSTEM_COMPONENTS];              // The types this match is matching against
  fcc_type_t                    m_has_components[FCC_MAX_HAS_COMPONENTS];               // The types of the "has" components
  fcc_type_t                    m_has_not_components[FCC_MAX_HAS_NOT_COMPONENTS];           // The types of the "has_not" components
  const char*                   m_has_tags[FCC_MAX_HAS_TAGS];                     // The "with" tags  
  const char*                   m_has_not_tags[FCC_MAX_HAS_NOT_TAGS];                 // The "has_not" tags
  fcc_decl_t                    m_filter_func[FCC_MAX_FILTER_FUNC];                  // The filter function
  uint32_t                      m_nfuncs;
  uint32_t                      m_ncmatches; 
  uint32_t                      m_nhas_components;
  uint32_t                      m_nhas_not_components;
  uint32_t                      m_nhas_tags;
  uint32_t                      m_nhas_not_tags;
  bool                          m_from_expand;                  // True if this comes from an expand
  char                          m_ref_name[FCC_MAX_REF_NAME];       // The reference name if comes form an expand
};


fcc_entity_match_t
fcc_entity_match_init();

/**
 * \brief releases a fcc_entity_match_t
 *
 * \param fcc_entity_match The fcc_entity_match_t to release
 */
void
fcc_entity_match_release(fcc_entity_match_t* fcc_entity_match);

void
fcc_entity_match_add_cmatch(fcc_entity_match_t* ematch, 
                            fcc_component_match_t cmatch);

void
fcc_entity_match_add_has_comp(fcc_entity_match_t* ematch, 
                              fcc_type_t comp);

void
fcc_entity_match_add_has_not_comp(fcc_entity_match_t* ematch, 
                                  fcc_type_t comp);

void
fcc_entity_match_add_has_tag(fcc_entity_match_t* ematch, 
                             const char* tag);

void
fcc_entity_match_add_has_not_tag(fcc_entity_match_t* ematch, 
                             const char* tag);

void
fcc_entity_match_add_filter(fcc_entity_match_t* ematch, 
                            fcc_decl_t fdecl);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

enum class fcc_stmt_place_t 
{
  E_FRAME = 0,
  E_POST_FRAME,
};

/**
 * \brief Structure used to store the information about a match stmt
 */
struct fcc_stmt_t
{
  fcc_operation_type_t            m_operation_type; // The type of operations
  fcc_ematch_ptr_array_t          m_ematches; // The set of entity matches that conform this query
  fcc_system_t*                   p_system;         // The system that executes on the results of this match
  fcc_expr_t                      m_expr;           // The expression of this match
  uint32_t                        m_priority;       // The execution priority of this match
  fcc_stmt_place_t               m_place;          // Where this match code should be output
};

/**
 * \brief inits an fcc_stmt_t 
 *
 * \return Returns a pointer to the initd fcc_stmt_t
 */
fcc_stmt_t
fcc_stmt_init();

void
fcc_stmt_release(fcc_stmt_t* fcc_stmt);


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * \brief Enum class for the different types of errors caputred by the FCC
 * compiler
 */
enum class fcc_parsing_error_type_t
{
  E_UNKNOWN_ERROR,
  E_UNKNOWN_FDB_OPERATION,
  E_INCOMPLETE_FDB_STATEMENT,
  E_UNSUPPORTED_VAR_DECLARATIONS,
  E_UNSUPPORTED_STATEMENT,
  E_UNSUPPORTED_TYPE_MODIFIER,
  E_EXPECTED_STRING_LITERAL,
  E_NO_ERROR,
};

enum class fcc_compilation_error_type_t
{
  E_UNKNOWN_ERROR,
  E_CYCLIC_DEPENDENCY_GRAPH,
  E_INVALID_COLUMN_TYPE,
  E_INVALID_ACCESS_MODE_ON_EXPAND,
  E_SYSTEM_INVALID_NUMBER_COMPONENTS,
  E_NO_ERROR,
};

typedef void (*FCC_PARSING_ERROR_CALLBACK)(fcc_parsing_error_type_t, 
                                           const char*, 
                                           int32_t, 
                                           int32_t,
                                           const char*);

typedef  void (*FCC_COMP_ERROR_CALLBACK)(fcc_compilation_error_type_t, 
                                         const char*);
/**
 * \brief Used to store the Fcc compilation context information
 */
struct fcc_context_t 
{
  FCC_PARSING_ERROR_CALLBACK  p_pecallback;             // Pointer to function handling parsing errors
  FCC_COMP_ERROR_CALLBACK     p_cecallback;             // Pointer to the function handling compilation errors

  fcc_stmt_ptr_array_t        m_stmts;          // The furious stmts extracted from the input code
  fcc_decl_array_t            m_using_decls;
};

/**
 * \brief Sets the parsing error callback function
 *
 * \param context The fcc_context to set the function for
 * \param callback The callback function to set
 */
void 
fcc_context_set_parsing_error_callback(FCC_PARSING_ERROR_CALLBACK e_callback);

/**
 * \brief Sets the compilation error callback function
 *
 * \param context The fcc_context to set the function for
 * \param callback The callback function to set
 */
void 
fcc_context_set_compilation_error_callback(FCC_COMP_ERROR_CALLBACK e_callback);

/**
 * \brief Reports a parsing error using the callback set in fcc_context
 *
 * \param context The context to use
 * \param error_type The type of error
 */
void 
FCC_CONTEXT_REPORT_PARSING_ERROR(fcc_parsing_error_type_t error_type,
                                 const char* filename,
                                 int32_t line,
                                 int32_t column,
                                 const char* message, 
                                 ...);

/**
 * \brief Reports a parsing error using the callback set in fcc_context
 *
 * \param context The context to use
 * \param error_type The type of error
 */
void 
FCC_CONTEXT_REPORT_COMPILATION_ERROR(fcc_compilation_error_type_t error_type,
                                     const char* err_msg, 
                                     ...);

///**
// * \brief inits an FccExecInfo in this context
// *
// * \param The ASTContext this execution info refers to
// *
// * \return The initd FccExecInfo.
// */
//fcc_stmt_t*
//fcc_context_init_match(ASTContext* ast_context, 
//                         Expr* expr);

/**
 * \brief Initializes the given fcc_context
 *
 * \param context The context to initialize
 */
void
fcc_context_init();

/**
 * \brief Releases the given fcc_context
 *
 * \param context The context to release 
 */
void 
fcc_context_release();


/**
 * \brief Runs the FCC compiler within the provided context and with the given
 * options
 *
 * \param argc the argument count
 * \param argv the arguments
 *
 * \return Returns the exit code of the compilation. 0 if successful.
 */
int 
fcc_run(int argc,
        const char** argv);

/**
 * \brief Validates the correctness of a match expression
 *
 * \param match The match expression to validate the correctness for
 *
 */
void
fcc_validate(const fcc_stmt_t* match);

#endif
