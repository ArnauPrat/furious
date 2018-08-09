
include_directories(./)
add_subdirectory(./common/)
add_subdirectory(./memory/)
add_subdirectory(./data/)
add_subdirectory(./runtime/)

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

add_executable( furious_test 
  furious_test.cpp
  )

add_executable( mvp_test 
  mvp_test.cpp
  )

target_link_libraries( furious_test furious memory)

set_target_properties(furious_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
)

add_subdirectory(./test/)

SET(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)


install (TARGETS furious
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.h"
       )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/ DESTINATION include/furious
       FILES_MATCHING PATTERN "*.inl"
       )
