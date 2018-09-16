
include_directories(./)

add_executable( fcc 
  backend/produce_visitor.h
  backend/consume_visitor.h
  backend/produce_visitor.cpp
  backend/consume_visitor.cpp
  clang_tools.h
  clang_tools.cpp
  backend/codegen.cpp
  backend/codegen.h
  frontend/execution_plan.cpp
  frontend/execution_plan.h
  frontend/execution_plan.inl
  frontend/exec_plan_printer.h
  frontend/exec_plan_printer.cpp
  fcc.cpp
  frontend/fccASTVisitor.cpp
  frontend/fccASTVisitor.h
  frontend/parsing.cpp
  frontend/parsing.h
  structs.cpp
  structs.h
  frontend/transforms.cpp
  frontend/transforms.h
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
