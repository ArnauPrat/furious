
include_directories(./)

add_executable( fcc 
  backend/codegen.cpp
  backend/codegen.h
  backend/codegen_tools.cpp
  backend/codegen_tools.h
  backend/consume_visitor.cpp
  backend/consume_visitor.h
  backend/produce_visitor.cpp
  backend/produce_visitor.h
  clang_tools.cpp
  clang_tools.h
  fcc.cpp
  frontend/exec_plan_printer.cpp
  frontend/exec_plan_printer.h
  frontend/execution_plan.cpp
  frontend/execution_plan.h
  frontend/execution_plan.inl
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

#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 
#FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

target_link_libraries(fcc 
  #  ${Boost_LIBRARIES} 
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
