
include_directories(./)

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

add_executable( fcc 
  ${COMMON_SRC}
  ${COMPILER_SRC}
  ${DRIVER_SRC}
  )

set(LLVM_LINK_COMPONENTS support)

link_directories(${LIBCLANG_LIBDIR})
target_link_libraries(fcc 
  ${LIBCLANG_LIBRARIES}
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
