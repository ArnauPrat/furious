#ifndef _FURIOUS_DRIVER_H
#define _FURIOUS_DRIVER_H value

#include "../common/platform.h"
#include "../runtime/data/reflection.h"
#include "fcc_context.h"

namespace furious
{

struct fcc_context_t;
struct FccExecPlan;

struct Dependency 
{
  char            m_include_file[FCC_MAX_INCLUDE_PATH_LENGTH];
  fcc_decl_t      m_decl;
};

void
fcc_driver_init();

void
fcc_driver_release();

int
fcc_parse_scripts(int argc, 
                  const char** argv,
                  fcc_context_t* fcc_context);

/**
 * \brief Gets the name of a fcc_type_t
 *
 * \param type The fcc_type_t to get the name from
 * \param buffer The buffer where the name will be written to 
 * \param type The length of the buffer 
 *
 * \return The length of the type name
 */
uint32_t
fcc_type_name(fcc_type_t type,
              char* buffer,
              uint32_t buffer_length);

/**
 * \brief Gets the qualified name of a fcc_type_t
 *
 * \param type The fcc_type_t to get the name from
 * \param buffer The buffer where the name will be written to 
 * \param type The length of the buffer 
 *
 * \return The length of the type name
 */
uint32_t
fcc_type_qualified_name(fcc_type_t type,
                        char* buffer,
                        uint32_t buffer_length);


/**
 * \brief Gets the access mode of the component represented by the given
 * fcc_type_t
 *
 * \param type The QualType representing the component
 *
 * \return The access mode of the component
 */
fcc_access_mode_t
fcc_type_access_mode(fcc_type_t type);

/**
 * \brief Gets the declaration of a fcc_type_t
 *
 * \param type The fcc_type_t to get the declaration from
 *
 * \return Returns true if the declaration could be found
 */
bool
fcc_type_decl(fcc_type_t type, fcc_decl_t* decl);

/**
 * \brief Gets the dependencies of a QualType 
 *
 * \param type The type to get the dependencies from
 *
 * \return Returns a dynamic array with the found dependencies
 */
DynArray<Dependency> 
fcc_type_dependencies(fcc_type_t type);


/**
 * \brief Gets the type of a declaration
 *
 * \param decl The declaration to get the type of
 * \param type The returned type
 */
void
fcc_decl_type(fcc_decl_t decl, fcc_type_t* type);

/**
 * \brief Gets the number of parameters for this declaration, in case it is a
 * function declaration
 *
 * \param decl The declaration to get the number of parameters for
 *
 * \return The number of parameters if the provided declaration is a function
 * declaration
 */
uint32_t
fcc_decl_function_num_params(fcc_decl_t decl);

/**
 * \brief Gets the type of a function parameter
 *
 * \param decl The function declaration
 * \param i The index of the parameter to get
 * \param type A pointer to set to type to
 *
 * \return True if the given parameter actually exists
 */
bool
fcc_decl_function_param_type(fcc_decl_t decl, 
                             uint32_t i, 
                             fcc_type_t* type);

/**
 * \brief Gets the name of the parameter
 *
 * \param decl The funciton declaration to get the parameter for
 * \param i The index of the parameter to get
 * \param buffer The buffer to store the parameter to
 * \param buffer_length The length of the buffer
 *
 * \return  The length of the parameter name
 */
uint32_t
fcc_decl_function_param_name(fcc_decl_t decl, 
                             uint32_t i, 
                             char* buffer,
                             uint32_t buffer_length);

/**
 * \brief Returns true if the declaration handler is valid
 *
 * \param decl The declaration handler to check
 *
 * \return True if the handler is valid. False otherwise
 */
bool
fcc_decl_is_valid(fcc_decl_t decl);

/**
 * \brief Returns true if the declaration is a variable or a struct declaration.
 * false otherwise
 *
 * \param decl The declaration to check
 *
 * \return True if it is a variable or struct declaration
 */
bool
fcc_decl_is_variable_or_struct(fcc_decl_t decl);

/**
 * \brief Returns true if the declaration is a function (non-class member)
 * delcaration
 *
 * \param decl The declaration to check
 *
 * \return true if the declaration is a function. flase otherwise
 */
bool
fcc_decl_is_function(fcc_decl_t decl);

/**
 * \brief Returns true if the declaration is a lambda expression 
 *
 * \param decl The declaration to check for
 *
 * \return Returns true if the declartion is a lambda expression. false
 * otherwise
 */
bool
fcc_decl_is_member(fcc_decl_t decl);

/**
 * \brief Gets the code of a declaration
 *
 * \param decl The declaration to get the code for
 * \param buffer The buffer to store the code to
 * \param buffer_length The length of the buffer
 *
 * \return Returns the total length of the code (which can be larger than buffer 
 * length, in such case not all the code is returned in the buffer). Returns 0
 * if the code is not available.
 */
uint32_t
fcc_decl_code(fcc_decl_t decl,
              char* buffer, 
              uint32_t buffer_length);

/**
 * \brief Gets the name of a function declaration
 *
 * \param decl The declaration to get the name from
 * \param buffer The buffer to store the name
 * \param buffer_length The length of the buffer
 *
 * \return Returns the length of the name (which can be larger than buffer 
 * length, in such case not all the code is returned in the buffer). Returns 0
 * if the declaration is not a function.
 */
uint32_t
fcc_decl_function_name(fcc_decl_t decl,
                       char* buffer, 
                       uint32_t buffer_length);

/**
 * \brief Returns true if the code of the given declaration is available
 *
 * \param decl The declaration to check the code for
 *
 * \return Returns true if the code is available, false otherwise
 */
bool
fcc_decl_code_available(fcc_decl_t decl);

/**
 * @brief Gets the dependencies of a given declaration
 *
 * @param decl The declaration to get the dependencies from
 *
 * @return Returns a dynamic array with the dependencies of a given declaration
 */
DynArray<Dependency> 
fcc_decl_dependencies(fcc_decl_t decl);

/**
 * \brief Checks if two declararations are the same
 *
 * \param decl_1 The first declaration to compare
 * \param decl_2 The second declaration to compare
 *
 * \return True if the two declarations are the same. False otherwise
 */
bool
fcc_decl_is_same(fcc_decl_t decl_1, fcc_decl_t decl_2);


/**
 * \brief Gets the code of an expression 
 *
 * \param epxr The expression to get the code for
 * \param buffer The buffer to store the code to
 * \param buffer_length The length of the buffer
 *
 * \return Returns the total length of the code (which can be larger than buffer 
 * length, in such case not all the code is returned in the buffer). Returns 0
 * if the code is not available.
 */
uint32_t
fcc_expr_code(fcc_expr_t expr,
              char* buffer, 
              uint32_t buffer_length);

/**
 * \brief Gets the location of an fcc_expr_t in the code
 *
 * \param expr The expression to get the location from 
 * \param filename_buffer The buffer to store the file name
 * \param filename_buffer_length The length of the buffer
 * \param line The line in the file where this expression is
 * \param column The column in the file where this expresion is
 *
 * \return  The length of the file name
 */
uint32_t 
fcc_expr_code_location(fcc_expr_t expr,
                       char* filename_buffer,
                       uint32_t filename_buffer_length,
                       uint32_t* line,
                       uint32_t* column);

/**
 * \brief Gets the reflection data structure for the provided declaration
 *
 * \param decl The declaration to get the reflection data from
 *
 * \return  Returns the reflection data of the given declaration 
 */
ReflData
get_reflection_data(fcc_decl_t decl);


} /* furious */ 
#endif /* ifndef _FURIOUS_DRIVER_H */
