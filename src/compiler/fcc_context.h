
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include "../common/dyn_array.h"
#include "../common/platform.h"

#include <string>

namespace furious 
{

struct fcc_context_t;
extern fcc_context_t* p_fcc_context;

struct fcc_type_t
{
  void* p_handler = nullptr;
};

struct fcc_expr_t
{
  void* p_handler = nullptr;
};

struct fcc_decl_t
{
  void* p_handler = nullptr;
};

enum class fcc_operation_type_t 
{
  E_UNKNOWN,
  E_FOREACH
};

enum class fcc_access_mode_t 
{
  E_READ,
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
  fcc_type_t                      m_system_type;                // The type of the system
  DynArray<fcc_expr_t>            m_ctor_params;                // The expressions of the system's constructor parameters 
  DynArray<fcc_component_match_t> m_component_types;            // The types of the components of the system
  int32_t                         m_id                    = -1; // The id of the system  
};


/**
 * \brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct fcc_entity_match_t 
{
  DynArray<fcc_component_match_t>     m_component_types;                  // The types this match is matching against
  DynArray<fcc_type_t>                m_has_components;               // The types of the "has" components
  DynArray<fcc_type_t>                m_has_not_components;           // The types of the "has_not" components
  DynArray<char*>                     m_has_tags;                     // The "with" tags  
  DynArray<char*>                     m_has_not_tags;                 // The "has_not" tags
  DynArray<fcc_decl_t>                m_filter_func;                  // The filter function
  bool                                m_from_expand;                  // True if this comes from an expand
  char                                m_ref_name[MAX_REF_NAME];       // The reference name if comes form an expand
};

/**
 * \brief Creates a fcc_entity_match_t
 *
 * \return The created fcc_entity_match
 */
fcc_entity_match_t* 
fcc_entity_match_create();

/**
 * \brief Destroys a fcc_entity_match_t
 *
 * \param fcc_entity_match The fcc_entity_match_t to destroy
 */
void
fcc_entity_match_destroy(fcc_entity_match_t* fcc_entity_match);

enum class fcc_match_place_t 
{
  E_FRAME,
  E_POST_FRAME,
};

/**
 * \brief Structure used to store the information about a match stmt
 */
struct fcc_stmt_t
{
  fcc_operation_type_t            m_operation_type; // The type of operations
  DynArray<fcc_entity_match_t*>   p_entity_matches; // The set of entity matches that conform this query
  fcc_system_t*                   p_system;         // The system that executes on the results of this match
  fcc_expr_t                      m_expr;           // The expression of this match
  uint32_t                        m_priority;       // The execution priority of this match
  fcc_match_place_t               m_place;          // Where this match code should be output
};

/**
 * \brief Creates an fcc_stmt_t 
 *
 * \return Returns a pointer to the created fcc_stmt_t
 */
fcc_stmt_t*
fcc_stmt_create();

void
fcc_stmt_destroy(fcc_stmt_t* fcc_stmt);


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
  E_UNKNOWN_FURIOUS_OPERATION,
  E_INCOMPLETE_FURIOUS_STATEMENT,
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

  DynArray<fcc_stmt_t*>               p_stmts;          // The furious stmts extracted from the input code
  DynArray<fcc_decl_t>                m_using_decls;
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
fcc_context_report_parsing_error(fcc_parsing_error_type_t error_type,
                                 const char* filename,
                                 int32_t line,
                                 int32_t column,
                                 const char* message);

/**
 * \brief Reports a parsing error using the callback set in fcc_context
 *
 * \param context The context to use
 * \param error_type The type of error
 */
void 
fcc_context_report_compilation_error(fcc_compilation_error_type_t error_type,
                                     const char* err_msg);

///**
// * \brief Creates an FccExecInfo in this context
// *
// * \param The ASTContext this execution info refers to
// *
// * \return The created FccExecInfo.
// */
//fcc_stmt_t*
//fcc_context_create_match(ASTContext* ast_context, 
//                         Expr* expr);

/**
 * \brief Initializes the given fcc_context
 *
 * \param context The context to initialize
 */
void
fcc_context_create();

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

} /* furious */ 

#endif
