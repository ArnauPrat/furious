
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(data STATIC 
  bit_table.cpp
  bit_table.h
  block_cluster.cpp
  block_cluster.h
  common.h
  context.cpp
  context.h
  context.inl 
  data.h
  database.cpp
  database.h
  database.inl
  entity.cpp
  entity.h
  table.cpp
  table.h
  table.inl 
  table_view.h
  table_view.inl 
)

target_link_libraries(data common memory)

set_target_properties( data 
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
