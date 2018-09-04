
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include <string>
#include <vector>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

namespace furious {

struct FccOperator;

enum class FccOperationType {
  E_UNKNOWN,
  E_FOREACH
};

struct FccSystemInfo 
{
  QualType                  m_system_type;  // The type of the system
  std::vector<const Expr*>  m_ctor_params;  // The expressions of the system's constructor parameters 
};

/**
 * @brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct FccExecInfo 
{
  ASTContext*           p_ast_context;              // clang ast context
  FccOperationType      m_operation_type = FccOperationType::E_UNKNOWN; // The type of operations
  FccSystemInfo         m_system;                   // The system to execute
  std::vector<QualType> m_basic_component_types;    // The types of the components of the system
  std::vector<QualType> m_has_components;          // The types of the "has" components
  std::vector<QualType> m_has_not_components;       // The types of the "has_not" components

  std::vector<std::string>  m_has_tags;            // The "with" tags  
  std::vector<std::string>  m_has_not_tags;         // The "has_not" tags

  std::vector<const FunctionDecl*> m_filter_func;   // The filter function
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * @brief Enum class for the different types of errors caputred by the FCC
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
};

/**
 * @brief Used to store the Fcc compilation context information
 */
struct FccContext {
  void (*p_pecallback)(FccContext*, 
                      FccParsingErrorType, 
                      const std::string&, 
                      int32_t, 
                      int32_t); // Pointer to function handling the error

  void (*p_cecallback)(FccContext*,
                       FccCompilationErrorType,
                       const FccOperator*);

  std::vector<std::unique_ptr<ASTUnit>> m_asts;       // Vector with the ASTs of all the translation units
  std::vector<FccExecInfo>              m_operations; // The furious operation info extracted from the input code
};

/**
 * @brief Initializes the given FccContext
 *
 * @param context The context to initialize
 */
FccContext*
FccContext_create_and_init();

/**
 * @brief Releases the given FccContext
 *
 * @param context The context to release 
 */
void 
FccContext_release(FccContext* context);

/**
 * @brief Sets the parsing error callback function
 *
 * @param context The FccContext to set the function for
 * @param callback The callback function to set
 */
void 
FccContext_set_parsing_error_callback(FccContext* context,
                                      void (*cback)(FccContext*, 
                                                    FccParsingErrorType, 
                                                    const std::string&, 
                                                    int32_t, 
                                                    int32_t));

/**
 * @brief Sets the compilation error callback function
 *
 * @param context The FccContext to set the function for
 * @param callback The callback function to set
 */
void 
FccContext_set_compilation_error_callback(FccContext* context,
                                          void (*cback)(FccContext*, 
                                                        FccCompilationErrorType, 
                                                        const FccOperator*));

/**
 * @brief Reports a parsing error using the callback set in FccContext
 *
 * @param context The context to use
 * @param error_type The type of error
 */
void 
FccContext_report_parsing_error(FccContext* context,
                                FccParsingErrorType error_type,
                                const std::string& filename,
                                int32_t line,
                                int32_t column);

/**
 * @brief Reports a parsing error using the callback set in FccContext
 *
 * @param context The context to use
 * @param error_type The type of error
 */
void 
FccContext_report_compilation_error(FccContext* context,
                                    FccCompilationErrorType error_type,
                                    const FccOperator* fcc_operator);

/**
 * @brief Runs the FCC compiler within the provided context and with the given
 * options
 *
 * @param context The context to run the compiler with
 * @param op The options to run the compiler with
 *
 * @return Returns the exit code of the compilation. 0 if successful.
 */
int 
FccContext_run(FccContext* context, 
               CommonOptionsParser& op);

} /* furious */ 

#endif
