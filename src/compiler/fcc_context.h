
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

struct FccOperator;
struct FccContext;

enum class FccOperationType 
{
  E_UNKNOWN,
  E_FOREACH
};

enum class AccessMode
{
  E_READ,
  E_READ_WRITE,
  E_WRITE
};

/**
 * \brief 
 */
struct FccSystemInfo 
{
  FccSystemInfo(FccContext* fcc_context);
  FccSystemInfo(const FccSystemInfo&) = delete;

  ~FccSystemInfo();

  /**
   * \brief Inserts a new constructor parameter expression
   *
   * \param param The expression to insert
   */
  void
  insert_ctor_param(const Expr* param);

  FccContext*               p_fcc_context;
  QualType                  m_system_type;        // The type of the system
  DynArray<const Expr*>     m_ctor_params;        // The expressions of the system's constructor parameters 
  int32_t                   m_id;
};

/**
 * \brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct FccExecInfo 
{
  FccExecInfo(ASTContext* ast_context,
              FccContext* fcc_context);

  FccExecInfo(const FccExecInfo&) = delete;

  ~FccExecInfo();

  /**
   * \brief Inserts a component type to this execution info
   *
   * \param q_type The component type to add
   */
  void
  insert_component_type(const QualType* q_type);

  /**
   * \brief Inerts a has component predicate to this execution info
   *
   * \param q_type The component of the predicate 
   */
  void
  insert_has_compponent(const QualType* q_type);

  /**
   * \brief Inserts a has not component predicate to this execution info
   *
   * \param q_type The component of the predicate 
   */
  void
  insert_has_not_component(const QualType* q_type);

  /**
   * \brief Inserts a has tag predicate to this execution info
   *
   * \param q_type The tag of the predicate
   */
  void
  insert_has_tag(const std::string& q_type);

  /**
   * \brief Inserts a has not tag predicate to this execution info
   *
   * \param q_type The tag of the predicate
   */
  void
  insert_has_not_tag(const std::string& q_type);

  /**
   * \brief Inserts a filter function predicate to this execution info
   *
   * \param decl The function declaration to add
   */
  void
  insert_filter_func(const FunctionDecl* decl);

  ASTContext*           p_ast_context;                 // clang ast context
  FccContext*           p_fcc_context;                 // the fcc context this exec info belongs to
  FccOperationType      m_operation_type;              // The type of operations
  FccSystemInfo         m_system;                      // The system to execute

  DynArray<QualType>            m_basic_component_types;       // The types of the components of the system
  DynArray<QualType>            m_has_components;              // The types of the "has" components
  DynArray<QualType>            m_has_not_components;          // The types of the "has_not" components
  DynArray<std::string>         m_has_tags;                    // The "with" tags  
  DynArray<std::string>         m_has_not_tags;                // The "has_not" tags
  DynArray<const FunctionDecl*> p_filter_func;                 // The filter function
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
  E_EXPECTED_STRING_LITERAL,
};

enum class FccCompilationErrorType
{
  E_UNKNOWN_ERROR,
  E_CYCLIC_DEPENDENCY_GRAPH
};

typedef void (*FCC_PARSING_ERROR_CALLBACK)(FccContext*, 
                                           FccParsingErrorType, 
                                           const std::string&, 
                                           int32_t, 
                                           int32_t);

typedef  void (*FCC_COMP_ERROR_CALLBACK)(FccContext*, 
                                         FccCompilationErrorType, 
                                         const FccOperator*);


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
                       int32_t column);

  /**
   * \brief Reports a parsing error using the callback set in FccContext
   *
   * \param context The context to use
   * \param error_type The type of error
   */
  void 
  report_compilation_error(FccCompilationErrorType error_type,
                           const FccOperator* fcc_operator);

  /**
   * \brief Creates an FccExecInfo in this context
   *
   * \param The ASTContext this execution info refers to
   *
   * \return The created FccExecInfo.
   */
  FccExecInfo*
  create_exec_info(ASTContext* ast_context);

  /**
   * \brief Inserts an ASTUnit to this context.
   *
   * \param unit The unit to insert.
   */
  void
  insert_ast_unit(std::unique_ptr<ASTUnit>& unit);


  /**
   * \brief Inserts a using declaration to this context
   *
   * \param decl The using declaration to add
   */
  void
  insert_using_decl(const UsingDirectiveDecl* using_decl);


  FCC_PARSING_ERROR_CALLBACK  p_pecallback;             // Pointer to function handling parsing errors
  FCC_COMP_ERROR_CALLBACK     p_cecallback;             // Pointer to the function handling compilation errors

  DynArray<std::unique_ptr<ASTUnit>>  p_asts;                   // Vector with the ASTs of all the translation units
  DynArray<const FccExecInfo*>        p_exec_infos;             // The furious execution infos extracted from the input code
  DynArray<const UsingDirectiveDecl*> p_using_decls;

};

/**
 * \brief Initializes the given FccContext
 *
 * \param context The context to initialize
 */
FccContext*
Fcc_create_context();

/**
 * \brief Releases the given FccContext
 *
 * \param context The context to release 
 */
void 
Fcc_release_context(FccContext* context);


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
Fcc_run(FccContext* context, 
        CommonOptionsParser& op,
        const std::string& output_file,
        const std::string& include_file);

} /* furious */ 

#endif
