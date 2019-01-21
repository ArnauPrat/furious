CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -O0 -g -pg ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG ")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -pg -DNDEBUG -ftree-vectorize -ftree-vectorizer-verbose=7 -fopt-info-vec-all -fno-devirtualize ")

add_definitions(${FURIOUS_DEFAULT_DEFINES})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${FURIOUS_INCLUDE_DIR})

function(create_test NAME)

  add_custom_command(
										OUTPUT ${NAME}_furious.cpp
                    COMMAND fcc -o
                    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_furious.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}_script.cpp -- --std=c++17 -I ${CLANG_INCLUDES_DIR} -I ${CMAKE_SOURCE_DIR}/src
                    DEPENDS fcc ${NAME}_script.cpp
    )

  add_executable(${NAME}
    ${NAME}.cpp
    ${NAME}_header.h
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_furious.cpp
    )

  target_link_libraries(${NAME} ${GTEST_LIBRARIES} furious furious_runtime) 
  add_test(${NAME} ${FURIOUS_OUTPUT_DIR}/test/compiler/${NAME})

  set_target_properties( ${NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}/test/compiler
    )
endfunction(create_test)

SET(TESTS 
    basic_test
    filter_tag_test
    filter_func_test
    filter_lambda_test
  )

foreach( TEST ${TESTS} )
  create_test(${TEST})
endforeach( TEST )

add_custom_target(compiler_tests COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TESTS})
