
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include <string>

//#include <clang/Frontend/ASTUnit.h>
//#include <clang/Driver/Options.h>
//#include <clang/AST/AST.h>
//#include <clang/AST/ASTConsumer.h>
//#include <clang/Rewrite/Core/Rewriter.h>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/AST.h>
#include <clang/Tooling/CommonOptionsParser.h>

using namespace clang;
//using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

namespace furious {

struct FccOperator;
struct FccContext;

#define FCC_MAX_TRANSLATION_UNITS   1024
#define FCC_MAX_OPERATIONS          2024
#define FCC_MAX_SYSTEM_CTOR_PARAMS  16
#define FCC_MAX_EXEC_INFO_CTYPES    8
#define FCC_MAX_EXEC_INFO_HAS       8
#define FCC_MAX_EXEC_INFO_FILTER    8 


enum class FccOperationType {
  E_UNKNOWN,
  E_FOREACH
};

/**
 * \brief 
 */
struct FccSystemInfo 
{
  FccSystemInfo(FccContext* fcc_context);

  void
  insert_ctor_param(const Expr* param);

  FccContext*               p_fcc_context;
  QualType                  m_system_type;                              // The type of the system
  uint32_t                  m_num_ctor_params;
  const Expr*               m_ctor_params[FCC_MAX_SYSTEM_CTOR_PARAMS];  // The expressions of the system's constructor parameters 
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
  void
  insert_component_type(const QualType* q_type);

  void
  insert_has_compponent(const QualType* q_type);

  void
  insert_has_not_component(const QualType* q_type);

  void
  insert_has_tag(const std::string& q_type);

  void
  insert_has_not_tag(const std::string& q_type);

  void
  insert_filter_func(const FunctionDecl* decl);

  ASTContext*           p_ast_context;                                  // clang ast context
  FccContext*           p_fcc_context;                                  // the fcc context this exec info belongs to
  FccOperationType      m_operation_type;                               // The type of operations
  FccSystemInfo         m_system;                                       // The system to execute

  uint32_t              m_num_basic_component_types;                    // Number of basic component types
  uint32_t              m_num_has_components;                           // Number of has components filters
  uint32_t              m_num_has_not_components;                       // Number of has not component filters
  uint32_t              m_num_has_tags;                                 // Number of has tag filters
  uint32_t              m_num_has_not_tags;                             // Number of has not tag filters
  uint32_t              m_num_func_filters;                             // Numbner of function based filters

  QualType              m_basic_component_types[FCC_MAX_EXEC_INFO_CTYPES];  // The types of the components of the system
  QualType              m_has_components[FCC_MAX_EXEC_INFO_HAS];            // The types of the "has" components
  QualType              m_has_not_components[FCC_MAX_EXEC_INFO_HAS];        // The types of the "has_not" components
  std::string           m_has_tags[FCC_MAX_EXEC_INFO_HAS];                  // The "with" tags  
  std::string           m_has_not_tags[FCC_MAX_EXEC_INFO_HAS];              // The "has_not" tags
  const FunctionDecl*   m_filter_func[FCC_MAX_EXEC_INFO_FILTER];            // The filter function
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
  E_MAX_SYSTEM_CTOR_PARAMS,
  E_MAX_EXEC_INFO_CTYPES,
  E_MAX_EXEC_INFO_HAS,
  E_MAX_EXEC_INFO_FILTER,
  E_MAX_TRANSLATION_UNIT,
  E_MAX_OPERATION,
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


  FCC_PARSING_ERROR_CALLBACK  p_pecallback;             // Pointer to function handling parsing errors
  FCC_COMP_ERROR_CALLBACK     p_cecallback;             // Pointer to the function handling compilation errors

  uint32_t                    m_num_translation_units;  // The number of translation units in this context
  uint32_t                    m_num_exec_infos;         // The number of execution info structures

  std::unique_ptr<ASTUnit>    m_asts[FCC_MAX_TRANSLATION_UNITS];    // Vector with the ASTs of all the translation units
  FccExecInfo**               m_exec_infos;                         // The furious execution infos extracted from the input code
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
