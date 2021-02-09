CMAKE_MINIMUM_REQUIRED (VERSION 2.8)


add_definitions(${FURIOUS_DEFAULT_DEFINES})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${FURIOUS_INCLUDE_DIR})

function(create_test NAME)

  SET(CXX_INCLUDES_DIRS "")
  foreach(INC ${CXX_HEADERS_DIRS})
    SET(CXX_INCLUDES_DIRS "-I${INC}" " " ${CXX_INCLUDES_DIRS})
  endforeach()
  #string(REGEX REPLACE "\\" "" CXX_INCLUDES_DIRS "${CXX_INCLUDES_DIRS}")

  #MESSAGE(STATUS ${CXX_INCLUDES_DIRS})
  add_custom_command(
										OUTPUT ${NAME}_furious.cpp
                    COMMAND fcc -o
                    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_furious.cpp -i
                    "furious.h"
                    ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}_script.cpp --
                    -fstrict-aliasing -std=c++17 ${CXX_INCLUDES_DIRS} -I${CMAKE_SOURCE_DIR}/src ${FURIOUS_DEFAULT_DEFINES}
                    COMMAND clang-format --style='{BasedOnStyle: LLVM, ColumnLimit: 120}' -i 
                    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_furious.cpp
                    DEPENDS fcc furious ${NAME}_script.cpp ${NAME}_header.h
    )

  add_executable(${NAME}
    ${NAME}.cpp
    ${NAME}_header.h
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_furious.cpp
    )

  target_link_libraries(${NAME} ${GTEST_LIBRARIES} furious) 
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
     dependency_test
     expand_test
#    global_test
#    reflection_test
  )

foreach( TEST ${TESTS} )
  create_test(${TEST})
endforeach( TEST )

add_custom_target(compiler_tests COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TESTS})
