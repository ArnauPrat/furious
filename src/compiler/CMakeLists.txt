
include_directories(./)

set(COMPILER_SRC
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen_tools.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/codegen_tools.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/consume_visitor.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/consume_visitor.h
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/produce_visitor.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/backend/produce_visitor.h
  ${CMAKE_SOURCE_DIR}/src/compiler/clang_tools.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/clang_tools.h
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc_context.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/fcc_context.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/dep_graph.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/dep_graph.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan_printer.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/exec_plan_printer.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/execution_plan.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/execution_plan.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/execution_plan.inl
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/fccASTVisitor.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/fccASTVisitor.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/parsing.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/parsing.h
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/transforms.cpp
  ${CMAKE_SOURCE_DIR}/src/compiler/frontend/transforms.h
  )

add_executable( fcc 
  ${COMMON_SRC}
  ${COMPILER_SRC}
  )

#set_target_properties(fcc
#  PROPERTIES
#  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#)

#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
#FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
set(LLVM_LINK_COMPONENTS support)

target_link_libraries(fcc 
  clang 
  clangAST 
  clangASTMatchers
  clangSerialization
  clangFrontend
  clangTooling
  clangRewrite
  clangBasic
  clangLex
  LLVM
  )

install (TARGETS fcc 
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib)

set(TIDY_SRC
    ${COMPILER_SRC}
    ${COMMON_SRC}
    )

list(FILTER TIDY_SRC EXCLUDE REGEX ".inl$")

add_custom_target(
  tidy-compiler
  COMMAND clang-tidy ${TIDY_SRC} ${COMPILER_SRC} -- --std=c++17 -I
  ${CLANG_INCLUDES_DIR} -I ${CMAKE_SOURCE_DIR}/src -x c++
  COMMENT "running clang tidy on COMPILER"
  )
