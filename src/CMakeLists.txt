
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
  ${CMAKE_SOURCE_DIR}/src/common/string_builder.cpp
  ${CMAKE_SOURCE_DIR}/src/common/string_builder.h
  ${CMAKE_SOURCE_DIR}/src/common/traits.h
  ${CMAKE_SOURCE_DIR}/src/common/types.h
  ${CMAKE_SOURCE_DIR}/src/common/utils.cpp
  ${CMAKE_SOURCE_DIR}/src/common/utils.h
  )

set (RUNTIME_SRC
  runtime/data/bit_table.cpp
  runtime/data/bit_table.h
  runtime/data/block_cluster.cpp
  runtime/data/block_cluster.h
  runtime/data/common.h
  runtime/data/context.cpp
  runtime/data/context.h
  runtime/data/data.h
  runtime/data/database.cpp
  runtime/data/database.h
  runtime/data/database.inl
  runtime/data/entity.cpp
  runtime/data/entity.h
  runtime/data/entity.inl
  runtime/data/macros.h
  runtime/data/table.cpp
  runtime/data/table.h
  runtime/data/table.inl
  runtime/data/table_view.h
  runtime/data/table_view.inl
  runtime/data/webserver/webserver.h
  runtime/data/webserver/webserver.cpp
  runtime/memory/memory.h
  runtime/memory/numa_alloc.cpp
  runtime/memory/numa_alloc.h
  runtime/runtime.h
  runtime/system_wrapper.h
  runtime/system_wrapper.inl
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

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.h"
       )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.inl"
       )
