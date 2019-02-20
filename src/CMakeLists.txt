
include_directories(./)

set (COMMON_SRC
  common/dyn_array.h
  common/dyn_array.inl
  common/bitmap.cpp
  common/bitmap.h
  common/btree.h
  common/btree.inl
  common/common.h
  common/hash_map.h
  common/hash_map.inl
  common/impl/btree_impl.cpp
  common/impl/btree_impl.h
  common/platform.cpp
  common/platform.h
  common/reference.h
  common/string_builder.cpp
  common/string_builder.h
  common/traits.h
  common/types.h
  common/utils.cpp
  common/utils.h
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

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#add_executable( lang_test 
#  lang_test.cpp
#  )

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
