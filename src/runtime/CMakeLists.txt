
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(runtime STATIC 
  #backend.h
  #backends/volcano/volcano.h
  #backends/volcano/volcano.cpp
  #backends/volcano/temp_buffer.h
  #backends/volcano/temp_buffer.cpp
  #backends/basic/basic.h
  #backends/basic/basic.cpp
  static_system.h
  static_system.inl
  execution_plan.h
  execution_plan.cpp
  system.h
  system.cpp
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
