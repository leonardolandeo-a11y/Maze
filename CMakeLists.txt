cmake_minimum_required(VERSION 3.20)
project(Maze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Install FTXUI
include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v7.0.3
)
FetchContent_MakeAvailable(ftxui)


# Include src files
add_library(Maze_core
    src/agent.cpp
    src/environment.cpp
    src/controllers.cpp
    src/game_rules.cpp
    src/console_ui.cpp
    src/types.cpp
    src/GameApplication.cpp
)
# Include headers
target_include_directories(Maze_core PUBLIC include)

# Create the executable game and add the libraries
add_executable(Maze 
    app/main.cpp
    src/console_ui.cpp
    src/GameApplication.cpp
)
target_link_libraries(Maze PRIVATE
    Maze_core
    ftxui::screen
    ftxui::dom
    ftxui::component
)


#Tests

# enable_testing()

# add_executable(Maze_test
#     tests/grid_test.cpp
# #     tests/environment_test.cpp
# #     tests/interactions_test.cpp
# #     tests/controllers_test.cpp
# )

# target_link_libraries(Maze_test PRIVATE Maze_core)

# add_test(NAME Maze_test COMMAND Maze_test)
