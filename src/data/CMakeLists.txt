
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(data STATIC 
  common.h
  context.h
  context.cpp
  context.inl 
  data.h
  database.cpp
  database.h
  database.inl
  entity.cpp
  entity.h
  table.cpp
  table.inl 
  table.h
  table_view.h
  table_view.inl 
)

target_link_libraries(data common memory)

set_target_properties( data 
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
