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
  add_executable(${NAME}
    ${NAME}.cpp
    data_test.h
    )

  target_link_libraries(${NAME} ${GTEST_LIBRARIES} common memory data runtime) 
  add_test(${NAME} ${FURIOUS_OUTPUT_DIR}/test/runtime/${NAME})

  set_target_properties( ${NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}/test/runtime
    )
endfunction(create_test)

SET(TESTS 
  memory_test
  table_test
  database_test
  tags_test
  traits_test
  )

foreach( TEST ${TESTS} )
  create_test(${TEST})
endforeach( TEST )

add_custom_target(runtime_tests COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TESTS})