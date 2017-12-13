
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

Find_Package(NUMA)

add_library(memory STATIC
  numa_alloc.h
  numa_alloc.cpp
)

set_target_properties( memory   
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
