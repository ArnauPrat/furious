CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT( furious )

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RELEASE' as none was specified.")
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/;${CMAKE_MODULE_PATH}")

## Checking for required packages

find_package(LLVM REQUIRED CONFIG)
set(LIBCLANG_INCLUDES_DIR "${LLVM_INCLUDE_DIR}")
set(LLVM_INCLUDES_DIR "${LLVM_INCLUDE_DIR}")
set(LIBCLANG_LIBS_DIR "${LLVM_LIBRARY_DIRS}")
#set(LIBCLANG_LIBS 
#  clang 
#  clangTooling 
#  clangAST
#  clangCodeGen
#  omp)
set(LIBCLANG_LIBS 
  "${LIBCLANG_LIBS_DIR}/libclang.so"
  "${LIBCLANG_LIBS_DIR}/libclang-cpp.so.10")
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS )

#find_package(LibClang REQUIRED)
#if(NOT LIBCLANG_FOUND)
#  message(FATAL_ERROR "LibClang could not be found")
#endif()


#set(LIBCLANG_INCLUDES_DIR "${LIBCLANG_LIBDIR}/clang/${LIBCLANG_VERSION_STRING}/include")

#if(NOT LIBCLANG_INCLUDE_DIR)
#  message(FATAL_ERROR "Need to define folder with clang includes")
#endif()

#set(CMAKE_INSTALL_PREFIX ${FURIOUS_INSTALL_PREFIX})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)



set(CMAKE_CXX_FLAGS 
  "${CMAKE_CXX_FLAGS} \
  -std=c++11 \
  -Wall \
  -fno-rtti \
  -fno-exceptions \
  -fstrict-aliasing \
  -Wstrict-aliasing")

set(CMAKE_C_FLAGS 
  "${CMAKE_C_FLAGS} \
  -std=c99 \
  -Wall \
  -fstrict-aliasing")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -pg -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -pg -ggdb -std=c++17")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -pg -DNDEBUG -ftree-vectorize -ftree-vectorizer-verbose=7 -fopt-info-vec-all -fno-devirtualize ")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ggdb -std=c++17")

IF(FDB_ENABLE_ASSERTS)
  set(FURIOUS_DEFAULT_DEFINES ${FURIOUS_DEFAULT_DEFINES} -DFDB_ENABLE_ASSERTS) 
ENDIF(FDB_ENABLE_ASSERTS)

IF(FDB_DEBUG)
  set(FURIOUS_DEFAULT_DEFINES ${FURIOUS_DEFAULT_DEFINES} -DFDB_DEBUG) 
ENDIF(FDB_DEBUG)

IF(UNIX)
  set(FURIOUS_DEFAULT_DEFINES 
    ${FURIOUS_DEFAULT_DEFINES}
    "-DFDB_OS_LINUX"
    "-DFDB_COMPILER_GCC")
ENDIF(UNIX)

IF( WIN32 )
ENDIF(WIN32)

add_definitions(${FURIOUS_DEFAULT_DEFINES})
set(FURIOUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(FURIOUS_OUTPUT_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/furious")

enable_testing()

message(STATUS "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "LLVM VERSION: ${LLVM_VERSION_MAJOR}")
message(STATUS "LLVM INCLUDE DIRS: ${LLVM_INCLUDES_DIR}")
message(STATUS "LLVM LIBS DIR: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM LIBS: ${LLVM_LIBS}")
message(STATUS "LIBCLANG INCLUDE DIR: ${LIBCLANG_INCLUDES_DIR}")
message(STATUS "LIBCLANG LIBS: ${LIBCLANG_LIBS}")
message(STATUS "LIBCLANG LIBS DIR: ${LIBCLANG_LIBS_DIR}")
message(STATUS "DEFINES: ${FURIOUS_DEFAULT_DEFINES}")

add_subdirectory(src)

