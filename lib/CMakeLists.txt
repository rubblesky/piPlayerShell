
add_library(terminal SHARED)
target_sources(
terminal
    PRIVATE
        terminal.c
    PUBLIC
        terminal.h  # poor PUBLIC example, see discussion below for why
)
set_target_properties(terminal
    PROPERTIES
        PUBLIC_HEADER
            ${CMAKE_CURRENT_SOURCE_DIR}/terminal.h
)
target_include_directories(terminal PUBLIC ${CMAKE_CURRENT_LIST_DIR})

find_package(Curses REQUIRED)
include_directories( ${NCURSES_INCLUDE_DIRS} )
target_link_libraries(terminal PRIVATE ${NCURSES_LIBRARIES})


add_library(sort SHARED)
target_sources(sort
    PRIVATE 
        sort.c
    PUBLIC
        sort.h

)
set_target_properties(sort
    PROPERTIES
        PUBLIC_HEADER
            ${CMAKE_CURRENT_SOURCE_DIR}/sort.h    
)        
target_include_directories(sort PUBLIC ${CMAKE_CURRENT_LIST_DIR})




#添加线程库
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(piPlayer PRIVATE Threads::Threads)




add_library(queue SHARED)
target_sources(queue
    PRIVATE 
        queue.c
    PUBLIC
        queue.h
)
set_target_properties(queue
    PROPERTIES
        PUBLIC_HEADER
            ${CMAKE_CURRENT_SOURCE_DIR}/queue.h    
)        
target_include_directories(queue PUBLIC ${CMAKE_CURRENT_LIST_DIR})
