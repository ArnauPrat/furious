
include_directories(./)

set (COMMON_SRC
  ${CMAKE_SOURCE_DIR}/src/common/memory/memory.h
  ${CMAKE_SOURCE_DIR}/src/common/memory/memory.c
  ${CMAKE_SOURCE_DIR}/src/common/memory/numa_alloc.c
  ${CMAKE_SOURCE_DIR}/src/common/memory/numa_alloc.h
  ${CMAKE_SOURCE_DIR}/src/common/memory/pool_allocator.c
  ${CMAKE_SOURCE_DIR}/src/common/memory/pool_allocator.h
  ${CMAKE_SOURCE_DIR}/src/common/memory/page_allocator.c
  ${CMAKE_SOURCE_DIR}/src/common/memory/page_allocator.h
  ${CMAKE_SOURCE_DIR}/src/common/memory/stack_allocator.c
  ${CMAKE_SOURCE_DIR}/src/common/memory/stack_allocator.h
  ${CMAKE_SOURCE_DIR}/src/common/mutex.h
  ${CMAKE_SOURCE_DIR}/src/common/mutex.c
  #${CMAKE_SOURCE_DIR}/src/common/refcount_ptr.h
  #${CMAKE_SOURCE_DIR}/src/common/refcount_ptr.inl
  #${CMAKE_SOURCE_DIR}/src/common/dyn_array.h
  #${CMAKE_SOURCE_DIR}/src/common/dyn_array.inl
  ${CMAKE_SOURCE_DIR}/src/common/bitmap.c
  ${CMAKE_SOURCE_DIR}/src/common/bitmap.h
  ${CMAKE_SOURCE_DIR}/src/common/btree.h
  ${CMAKE_SOURCE_DIR}/src/common/btree.c
  ${CMAKE_SOURCE_DIR}/src/common/common.h
  ${CMAKE_SOURCE_DIR}/src/common/platform.h
  ${CMAKE_SOURCE_DIR}/src/common/platform.c
  #${CMAKE_SOURCE_DIR}/src/common/reference.h
  ${CMAKE_SOURCE_DIR}/src/common/hashtable.c
  ${CMAKE_SOURCE_DIR}/src/common/hashtable.h
  ${CMAKE_SOURCE_DIR}/src/common/str_builder.c
  ${CMAKE_SOURCE_DIR}/src/common/str_builder.h
  ${CMAKE_SOURCE_DIR}/src/common/types.h
  ${CMAKE_SOURCE_DIR}/src/common/utils.c
  ${CMAKE_SOURCE_DIR}/src/common/utils.h
  )

set (RUNTIME_SRC
  ${CMAKE_SOURCE_DIR}/src/runtime/data/bittable.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/bittable.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/block_cluster.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/block_cluster.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/context.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/context.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data_utils.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/data_utils.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reflection.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/database.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/database.h
  #${CMAKE_SOURCE_DIR}/src/runtime/data/entity.c
  #${CMAKE_SOURCE_DIR}/src/runtime/data/entity.h
  #${CMAKE_SOURCE_DIR}/src/runtime/data/entity.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/ht_registry.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/ht_registry.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/macros.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/table.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reftable.c
  ${CMAKE_SOURCE_DIR}/src/runtime/data/reftable.h
  #${CMAKE_SOURCE_DIR}/src/runtime/data/table_view.h
  #${CMAKE_SOURCE_DIR}/src/runtime/data/table_view.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/data/webserver/webserver.h
  ${CMAKE_SOURCE_DIR}/src/runtime/data/webserver/webserver.c
  ${CMAKE_SOURCE_DIR}/src/runtime/runtime.h
  #${CMAKE_SOURCE_DIR}/src/runtime/system_wrapper.h
  #${CMAKE_SOURCE_DIR}/src/runtime/system_wrapper.inl
  ${CMAKE_SOURCE_DIR}/src/runtime/task/task.h
  ${CMAKE_SOURCE_DIR}/src/runtime/task/task.c
  ${CMAKE_SOURCE_DIR}/src/runtime/log/log.c
  )


add_library(furious SHARED
  furious.h
  furious.c
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
add_executable(lang_test 
               lang_test.cpp
              )

SET(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

install (TARGETS furious 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/src/runtime
        DESTINATION include/furious
        FILES_MATCHING PATTERN "*.h"
       )
