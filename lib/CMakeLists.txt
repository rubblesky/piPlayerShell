target_sources(
Player
    PRIVATE
        terminal.c
    PUBLIC
        terminal.h  # poor PUBLIC example, see discussion below for why
)


set(TERMINAL_SRC "terminal.c")
ADD_LIBRARY(terminal STATIC ${TERMINAL_SRC})
target_link_libraries(piPlayer terminal)