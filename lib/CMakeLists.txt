#project(terminal LANGUAGES C)
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
#set(TERMINAL_SRC "terminal.c")
#ADD_LIBRARY(terminal STATIC ${TERMINAL_SRC})
#target_link_libraries(piPlayer terminal)