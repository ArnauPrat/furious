

function(GENERATE_ARRAY 
         FDB_PREFIX 
         FDB_TYPE 
         FDB_FORWARD
         FDB_INCLUDE
         DST)
  string(TOUPPER ${FDB_PREFIX} FDB_GUARD)
  configure_file(${CMAKE_SOURCE_DIR}/src/common/templates/array.h.in
    ${DST}/${FDB_PREFIX}_array.h @ONLY)

  configure_file(${CMAKE_SOURCE_DIR}/src/common/templates/array.cpp.in
    ${DST}/${FDB_PREFIX}_array.cpp @ONLY)
endfunction(GENERATE_ARRAY)
