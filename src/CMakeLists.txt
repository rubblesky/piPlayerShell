SET(SRC_LIST "piPlayer.c" "dir.c" "err.c" )
SET(LIB_LIST "sort" "terminal")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_executable(piPlayer ${SRC_LIST})
target_sources(
piPlayer
    PRIVATE
    ${SRC_LIST}
)
target_link_libraries(
piPlayer 
    PUBLIC 
    ${LIB_LIST}
)
#target_link_libraries(piPlayer PUBLIC err dir)