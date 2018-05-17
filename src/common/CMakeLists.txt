
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(common STATIC 
  platform.cpp
  utils.cpp 
  optional.h
  platform.h
  reflection.h
  types.h
  reference.h
)

set_target_properties( common 
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  )
