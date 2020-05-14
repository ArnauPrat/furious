
include_directories(./)
include(${CMAKE_SOURCE_DIR}/cmake/modules/templates.cmake)

## GENERATING CONST CHAR ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "cchar_ptr" 
  "const char*"
  ""
  ""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

## GENERATING FCC_DECL ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "fcc_decl" 
  "void*"
  ""
  ""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

## GENERATING FCC_DEPCY ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "fcc_depcy" 
  "fcc_depcy_t"
  "struct fcc_depcy_t;"
  "#include \"../driver.h\""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

## GENERATING FCC_ENTITY_MATCH ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "fcc_ematch_ptr" 
  "fcc_entity_match_t*"
  "struct fcc_entity_match_t;"
  "#include \"../fcc_context.h\""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

## GENERATING CHAR ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "char" 
  "char"
  ""
  ""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

## GENERATING UINT32 ARRAY FROM TEMPLATES
GENERATE_ARRAY( 
  "uint32" 
  "uint32_t"
  ""
  ""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

GENERATE_ARRAY( 
  "fcc_stmt_ptr" 
  "fcc_stmt_t*"
  "struct fcc_stmt_t;"
  "#include \"../fcc_context.h\""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

GENERATE_ARRAY( 
  "fcc_decl" 
  "void*"
  ""
  ""
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/)

set(AUTOGEN_SRC
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/cchar_ptr_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/cchar_ptr_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_decl_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_decl_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_depcy_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_depcy_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_ematch_ptr_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_ematch_ptr_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/char_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/char_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/uint32_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/uint32_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_stmt_ptr_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_stmt_ptr_array.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_decl_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/autogen/fcc_decl_array.cpp
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCLANG_CXXFLAGS} -Wno-stringop-truncation")

set(DRIVER_SRC
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang_tools.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang_tools.h
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang_parsing.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang_parsing.h
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/clang_reflection.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/ast_visitor.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/drivers/clang/ast_visitor.h
  )

set(COMPILER_SRC
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen.h
  ${CMAKE_SOURCE_DIR}/src/compiler/driver.h
  ${CMAKE_SOURCE_DIR}/src/compiler/dyn_array.h
  ${CMAKE_SOURCE_DIR}/src/compiler/dyn_array.inl
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/reflection.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/reflection.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen_tools.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen_tools.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/consumer.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/consumer.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/producer.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/producer.h
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc_context.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc_context.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan_printer.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan_printer.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/operator.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/operator.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/transforms.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/transforms.h
  )

set(RUNTIME_SRC
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.h
  )

add_executable( fcc 
  ${AUTOGEN_SRC}
  ${COMMON_SRC}
  ${COMPILER_SRC}
  ${DRIVER_SRC}
  ${RUNTIME_SRC}
  )

set(LLVM_LINK_COMPONENTS support)

link_directories(${LIBCLANG_LIBDIR})
target_link_libraries(fcc 
  ${LIBCLANG_LIBRARIES}
  clang-cpp
  LLVM
  )


install (TARGETS fcc 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)

#set(TIDY_SRC
#    ${COMPILER_SRC}
#    ${COMMON_SRC}
#    )
#
#list(FILTER TIDY_SRC EXCLUDE REGEX ".inl$")
#
#add_custom_target(
#  tidy-compiler
#  COMMAND clang-tidy ${TIDY_SRC} ${COMPILER_SRC} -- --std=c++17 -I
#  ${CLANG_INCLUDES_DIR} -I ${CMAKE_SOURCE_DIR}/src -x c++
#  COMMENT "running clang tidy on COMPILER"
#  )
