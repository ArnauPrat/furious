
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(data STATIC
  btree.h
  btree.inl
  common.h
  data.h
  database.cpp
  database.h
  database.inl
  entity.cpp
  entity.h
  impl/btree_impl.cpp
  impl/btree_impl.h
  table.cpp
  table.inl 
  table.h
)

target_link_libraries(data common memory)

set_target_properties( data 
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
