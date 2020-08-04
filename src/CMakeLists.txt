SET(SRC_LIST "piPlayer.c")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
ADD_EXECUTABLE(piPlayer ${SRC_LIST})
target_sources(
piPlayer
    PRIVATE
    ${SRC_LIST}
)
target_link_libraries(piPlayer PUBLIC terminal)