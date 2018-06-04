
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(runtime STATIC 
  static_system.h
  static_system.inl
  execution_plan.h
  execution_plan.cpp
  system.h
  workload.h
  workload.cpp
  workload.inl
)
 
target_link_libraries(runtime data common)
 
set_target_properties(runtime 
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
