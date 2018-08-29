
include_directories(./)

add_executable( fcc 
  parsing.cpp
  parsing.h
  codegen.cpp
  codegen.h
  execution_plan.cpp
  execution_plan.h
  fcc.cpp
  fccASTVisitor.cpp
  fccASTVisitor.h
  structs.cpp
  structs.h
  transforms.cpp
  transforms.h
  transforms.h
  )

#set_target_properties(fcc
#  PROPERTIES
#  RUNTIME_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#  LIBRARY_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#  ARCHIVE_OUTPUT_DIRECTORY ${FURIOUS_OUTPUT_DIR}
#)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(fcc 
  ${Boost_LIBRARIES} 
  clang 
  clangAST 
  clangFrontend
  clangTooling
  clangRewrite
  clangBasic
  clangLex
  LLVM
  )

install (TARGETS fcc 
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib)
