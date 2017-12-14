CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -O0 -g -pg ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG ")

add_definitions(${FURIOUS_DEFAULT_DEFINES})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${FURIOUS_INCLUDE_DIR})

function(create_test NAME)
  add_executable(${NAME}
    ${NAME}.cpp
    data_test.h
    )

  target_link_libraries(${NAME} ${GTEST_LIBRARIES} furious) 
  add_test(${NAME} ${FURIOUS_OUTPUT_DIR}/test/${NAME})

  set_target_properties( ${NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}/test
    )
endfunction(create_test)

SET(TESTS "btree_test;memory_test;table_test;database_test;system_test")

foreach( TEST ${TESTS} )
  create_test(${TEST})
endforeach( TEST )

add_custom_target(tests COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TESTS})

#create_test ("btree_test")
#create_test ("memory_test")
#create_test ("table_test")
#create_test ("database_test")
#create_test ("system_test")
#create_test ("physical_scan_test")
#create_test ("physical_filter_test")
#create_test ("physical_hash_join_test")
#create_test ("physical_map_test")
#create_test ("execution_engine_test")
#create_test ("physical_plan_generator_test")

