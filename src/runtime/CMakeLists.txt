
add_definitions(${FURIOUS_DEFAULT_DEFINES})
include_directories(${FURIOUS_INCLUDE_DIR})

add_library(runtime STATIC
  #execution_engine.cpp
  #execution_engine.h
  #logic/logic_filter.h
  #logic/logic_filter.cpp
  #logic/logic_join.h
  #logic/logic_join.cpp
  #logic/logic_map.h
  #logic/logic_map.cpp
  #logic/logic_plan.h
  #logic/logic_scan.h
  #logic/logic_scan.cpp
  #logic/logic_plan_visitor.h
  #physical/physical_filter.cpp
  #physical/physical_filter.h
  #physical/physical_hash_join.cpp
  #physical/physical_hash_join.h
  #physical/physical_map.cpp
  #physical/physical_map.h
  #physical/physical_plan.h
  #physical/physical_scan.cpp
  #physical/physical_scan.h
  #physical_plan_generator.cpp
  static_system.h
  static_system.inl
  system.h
  workload.h
  workload.cpp
  workload.inl
)

set_target_properties(runtime 
 PROPERTIES
 LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
 ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)
