
include_directories(./)
add_subdirectory(./common/)
add_subdirectory(./runtime/)
add_subdirectory(./runtime/memory/)
add_subdirectory(./runtime/data/)
add_subdirectory(./compiler)

add_library(furious SHARED
  furious.h
  furious.cpp
  )  

target_link_libraries(furious common memory data runtime)

set_target_properties(furious 
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)

######################

add_library(furious_runtime SHARED
  furious_runtime.h
  furious_runtime.cpp
  )  

target_link_libraries(furious_runtime furious)

set_target_properties(furious_runtime
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)

add_executable( mvp_test 
  mvp_test.cpp
  )

add_executable( bitset_test
  bitset_test.cpp 
  )

add_executable( lang_test 
  lang_test.cpp
  )

add_subdirectory(./test/runtime)
add_subdirectory(./test/compiler)

SET(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)


install (TARGETS furious 
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib)

install (TARGETS furious_runtime
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.h"
       )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.inl"
       )
