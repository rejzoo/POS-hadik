cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(COMMON_SOURCES_SERVER
    snake.c
)

set(COMMON_SOURCES_CLIENT
    menu.c
)

add_executable(SnakeGameServer
    server.c
    ${COMMON_SOURCES_SERVER}
)
target_include_directories(SnakeGameServer PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(SnakeGameClient
    client.c
    ${COMMON_SOURCES_CLIENT}
)
target_include_directories(SnakeGameClient PRIVATE ${CMAKE_SOURCE_DIR})

# Link libraries (e.g., ncurses) for both executables
find_package(Curses REQUIRED)
target_link_libraries(SnakeGameServer ${CURSES_LIBRARIES})
target_link_libraries(SnakeGameClient ${CURSES_LIBRARIES})

