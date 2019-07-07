CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT( furious )

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release)
endif()

## Checking for required packages

#find_package(Clang REQUIRED)

set(CLANG_INCLUDES_DIR "/usr/lib/clang/8.0.0/include")

if(NOT CLANG_INCLUDES_DIR)
  message(FATAL_ERROR "Need to define folder wich clang includes")
endif()


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/;${CMAKE_MODULE_PATH}")
#set(CMAKE_INSTALL_PREFIX ${FURIOUS_INSTALL_PREFIX})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(DEFAULT_DEFINES ${DEFAULT_DEFINES} " -D_FILE_OFFSET_BITS=64") 
add_definitions(${DEFAULT_DEFINES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fno-rtti")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -pg -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -pg -DNDEBUG -ftree-vectorize -ftree-vectorizer-verbose=7 -fopt-info-vec-all -fno-devirtualize ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ggdb")

IF( UNIX )
  SET(PLATFORM "_LINUX")
ENDIF(UNIX)

IF( WIN32 )
  SET(PLATFORM "_WINDOWS")
ENDIF(WIN32)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${PLATFORM}")

set(FURIOUS_DEFAULT_DEFINES " ")
set(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FURIOUS_OUTPUT_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/furious")


enable_testing()

add_subdirectory(src)

