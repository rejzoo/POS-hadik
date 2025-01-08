cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

# Add common sources
set(COMMON_SOURCES
    snake.c
    menu.c
)

# Add the server executable
add_executable(SnakeGameServer
    server.c
    ${COMMON_SOURCES}
)
target_include_directories(SnakeGameServer PRIVATE ${CMAKE_SOURCE_DIR})

# Add the client executable
add_executable(SnakeGameClient
    client.c
    ${COMMON_SOURCES}
)
target_include_directories(SnakeGameClient PRIVATE ${CMAKE_SOURCE_DIR})

# Link libraries (e.g., ncurses) for both executables
find_package(Curses REQUIRED)
target_link_libraries(SnakeGameServer ${CURSES_LIBRARIES})
target_link_libraries(SnakeGameClient ${CURSES_LIBRARIES})

