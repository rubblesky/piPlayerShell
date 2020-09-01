SET(SRC_LIST "piPlayer.c" "dir.c" "err.c" "interface.c" )
SET(LIB_LIST "sort" "terminal")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_executable(piPlayer ${SRC_LIST})
target_sources(
piPlayer
    PRIVATE
    ${SRC_LIST}
)


#添加线程库
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(piPlayer PRIVATE Threads::Threads)

target_link_libraries(
piPlayer 
    PUBLIC 
    ${LIB_LIST}
)
find_package(Curses REQUIRED)
include_directories( ${NCURSESW_INCLUDE_DIRS} )
target_link_libraries(piPlayer PRIVATE ${NCURSESW_LIBRARIES})
set(CMAKE_C_FLAGS "-lncursesw")