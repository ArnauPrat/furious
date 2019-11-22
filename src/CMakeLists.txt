
include_directories(./)

set (COMMON_SRC
  ${CMAKE_SOURCE_DIR}/src/common/refcount_ptr.h
  ${CMAKE_SOURCE_DIR}/src/common/refcount_ptr.inl
  ${CMAKE_SOURCE_DIR}/src/common/dyn_array.h
  ${CMAKE_SOURCE_DIR}/src/common/dyn_array.inl
  ${CMAKE_SOURCE_DIR}/src/common/bitmap.cpp
  ${CMAKE_SOURCE_DIR}/src/common/bitmap.h
  ${CMAKE_SOURCE_DIR}/src/common/btree.h
  ${CMAKE_SOURCE_DIR}/src/common/btree.inl
  ${CMAKE_SOURCE_DIR}/src/common/common.h
  ${CMAKE_SOURCE_DIR}/src/common/hash_map.h
  ${CMAKE_SOURCE_DIR}/src/common/hash_map.inl
  ${CMAKE_SOURCE_DIR}/src/common/impl/btree_impl.cpp
  ${CMAKE_SOURCE_DIR}/src/common/impl/btree_impl.h
  ${CMAKE_SOURCE_DIR}/src/common/platform.cpp
  ${CMAKE_SOURCE_DIR}/src/common/platform.h
  ${CMAKE_SOURCE_DIR}/src/common/reference.h
  ${CMAKE_SOURCE_DIR}/src/common/str_builder.cpp
  ${CMAKE_SOURCE_DIR}/src/common/str_builder.h
  ${CMAKE_SOURCE_DIR}/src/common/types.h
  ${CMAKE_SOURCE_DIR}/src/common/utils.cpp
  ${CMAKE_SOURCE_DIR}/src/common/utils.h
  )

set (RUNTIME_SRC
  ${CMAKE_SOURCE_DIR}/src/runtime/data/bit_table.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/bit_table.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/block_cluster.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/block_cluster.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/common.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/context.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/context.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data_utils.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data_utils.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/database.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/database.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/database.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/entity.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/entity.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/entity.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/ht_registry.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/ht_registry.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/macros.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table_view.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table_view.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/webserver/webserver.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/webserver/webserver.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/memory/memory.h
  ${CMAKE_SOURCE_DIR}/src/runtime/memory/memory.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/memory/numa_alloc.cpp
  ${CMAKE_SOURCE_DIR}/src/runtime/memory/numa_alloc.h
  ${CMAKE_SOURCE_DIR}/src/runtime/runtime.h
  ${CMAKE_SOURCE_DIR}/src/runtime/system_wrapper.h
  ${CMAKE_SOURCE_DIR}/src/runtime/system_wrapper.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/task/task.h
  ${CMAKE_SOURCE_DIR}/src/runtime/task/task.cpp
  )


add_library(furious SHARED
  furious.h
  furious.cpp
  ${COMMON_SRC}
  ${RUNTIME_SRC}
  )  

set_property(TARGET furious PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(furious 
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)

######################
#### FCC COMPILER ####
######################

add_subdirectory(./compiler)

#######################
#######  TESTS  #######
#######################

add_subdirectory(./test/runtime)
add_subdirectory(./test/compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
add_executable( lang_test 
  lang_test.cpp
  )

SET(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

install (TARGETS furious 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ 
        DESTINATION include/furious
        FILES_MATCHING PATTERN "*.h"
       )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ 
        DESTINATION include/furious
        FILES_MATCHING PATTERN "*.inl"
       )

     #set(TIDY_SRC
     #    ${RUNTIME_SRC}
     #    ${COMMON_SRC}
     #    )
     #
     #list(FILTER TIDY_SRC EXCLUDE REGEX ".inl$")
     #
     #add_custom_target(
     #  tidy-runtime
     #  COMMAND clang-tidy ${TIDY_SRC} -checks=llvm-* -fix -- -std=c++17 -I ${CLANG_INCLUDES_DIR} -I ${CMAKE_SOURCE_DIR}/src -x c++
     #  COMMENT "running clang tidy on RUNTIME"
     #  )
