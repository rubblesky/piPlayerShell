
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