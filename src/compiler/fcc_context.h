
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include "../common/dyn_array.h"

#include <string>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/AST.h>
#include <clang/Tooling/CommonOptionsParser.h>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

namespace furious 
{

struct FccContext;

extern FccContext* p_fcc_context;

enum class FccOperationType 
{
  E_UNKNOWN,
  E_FOREACH
};

enum class FccAccessMode
{
  E_READ,
  E_READ_WRITE,
};

struct FccMatchType
{
  QualType  m_type;
  bool      m_is_read_only;
  bool      m_is_global;
};

/**
 * \brief 
 */
struct FccSystem
{
  QualType                  m_system_type;                // The type of the system
  DynArray<const Expr*>     m_ctor_params;                // The expressions of the system's constructor parameters 
  DynArray<FccMatchType>    m_match_types;                // The types of the components of the system
  int32_t                   m_id                    = -1; // The id of the system  
};


/**
 * \brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct FccEntityMatch 
{
  DynArray<FccMatchType>        m_match_types;                  // The types this match is matching against
  DynArray<QualType>            m_has_components;               // The types of the "has" components
  DynArray<QualType>            m_has_not_components;           // The types of the "has_not" components
  DynArray<std::string>         m_has_tags;                     // The "with" tags  
  DynArray<std::string>         m_has_not_tags;                 // The "has_not" tags
  DynArray<const FunctionDecl*> p_filter_func;                  // The filter function
  bool                          m_from_expand       = false;    // True if this comes from an expand
  std::string                   m_ref_name;                     // The reference name if comes form an expand
};

enum class FccMatchPlace
{
  E_FRAME,
  E_POST_FRAME,
};

/**
 * \brief Structure used to store the information about a match statement
 */
struct FccMatch
{
  ~FccMatch();

  FccEntityMatch* 
  create_entity_match();

  ASTContext*                     p_ast_context     = nullptr;                      // clang ast context
  FccOperationType                m_operation_type  = FccOperationType::E_UNKNOWN;  // The type of operations
  DynArray<FccEntityMatch*>       p_entity_matches;                                 // The set of entity matches that conform this query
  FccSystem*                      p_system          = nullptr;                      // The system that executes on the results of this match
  Expr*                           p_expr            = nullptr;                      // The clang expression of this match
  uint32_t                        m_priority        = 1;                            // The execution priority of this match
  FccMatchPlace                   m_place           = FccMatchPlace::E_FRAME;       // Where this match code should be output
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * \brief Enum class for the different types of errors caputred by the FCC
 * compiler
 */
enum class FccParsingErrorType
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

enum class FccCompilationErrorType
{
  E_UNKNOWN_ERROR,
  E_CYCLIC_DEPENDENCY_GRAPH,
  E_INVALID_COLUMN_TYPE,
  E_INVALID_ACCESS_MODE_ON_EXPAND,
  E_SYSTEM_INVALID_NUMBER_COMPONENTS,
  E_NO_ERROR,
};

typedef void (*FCC_PARSING_ERROR_CALLBACK)(FccParsingErrorType, 
                                           const std::string&, 
                                           int32_t, 
                                           int32_t,
                                           const std::string&);

typedef  void (*FCC_COMP_ERROR_CALLBACK)(FccCompilationErrorType, 
                                         const std::string&);


/**
 * \brief Used to store the Fcc compilation context information
 */
struct FccContext 
{
  FccContext();
  ~FccContext();

  /**
   * \brief Sets the parsing error callback function
   *
   * \param context The FccContext to set the function for
   * \param callback The callback function to set
   */
  void 
  set_parsing_error_callback(FCC_PARSING_ERROR_CALLBACK e_callback);

  /**
   * \brief Sets the compilation error callback function
   *
   * \param context The FccContext to set the function for
   * \param callback The callback function to set
   */
  void 
  set_compilation_error_callback(FCC_COMP_ERROR_CALLBACK e_callback);

  /**
   * \brief Reports a parsing error using the callback set in FccContext
   *
   * \param context The context to use
   * \param error_type The type of error
   */
  void 
  report_parsing_error(FccParsingErrorType error_type,
                       const std::string& filename,
                       int32_t line,
                       int32_t column,
                       const std::string& message) const;

  /**
   * \brief Reports a parsing error using the callback set in FccContext
   *
   * \param context The context to use
   * \param error_type The type of error
   */
  void 
  report_compilation_error(FccCompilationErrorType error_type,
                           const std::string& err_msg) const;

  /**
   * \brief Creates an FccExecInfo in this context
   *
   * \param The ASTContext this execution info refers to
   *
   * \return The created FccExecInfo.
   */
  FccMatch*
  create_match(ASTContext* ast_context, 
               Expr* expr);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
 

  FCC_PARSING_ERROR_CALLBACK  p_pecallback;             // Pointer to function handling parsing errors
  FCC_COMP_ERROR_CALLBACK     p_cecallback;             // Pointer to the function handling compilation errors

  DynArray<std::unique_ptr<ASTUnit>>  p_asts;                   // Vector with the ASTs of all the translation units
  DynArray<FccMatch*>                 p_matches;             // The furious execution infos extracted from the input code
  DynArray<const UsingDirectiveDecl*> p_using_decls;
};

/**
 * \brief Initializes the given FccContext
 *
 * \param context The context to initialize
 */
void
Fcc_create_context();

/**
 * \brief Releases the given FccContext
 *
 * \param context The context to release 
 */
void 
Fcc_release_context();


/**
 * \brief Runs the FCC compiler within the provided context and with the given
 * options
 *
 * \param context The context to run the compiler with
 * \param op The options to run the compiler with
 * \param output_file The output file to generate 
 * \param include_file The furious include file path 
 *
 * \return Returns the exit code of the compilation. 0 if successful.
 */
int 
Fcc_run(CommonOptionsParser& op,
        const std::string& output_file,
        const std::string& include_file);

/**
 * \brief Validates the correctness of a match expression
 *
 * \param match The match expression to validate the correctness for
 *
 */
void
Fcc_validate(const FccMatch* match);

} /* furious */ 

#endif
