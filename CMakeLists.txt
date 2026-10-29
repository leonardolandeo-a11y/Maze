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
    src/help.cpp
    src/types.cpp
    src/GameApplication.cpp
    src/menus.cpp
    src/scenarios/scenario_1.cpp
    src/scenarios/scenario_2.cpp
    src/StartupAnimation.cpp 
    src/DeathAnimation.cpp
    src/VictoryAnimation.cpp
    src/map_generation/map_selector.cpp
        src/graph.cpp
    src/graph/disjoint_set.cpp
    src/map_generation/kruskal.cpp
    src/pathfinding/dijkstra.cpp
    src/map_generation/map_validator.cpp
    src/map_generation/game_verifier.cpp
    src/map_generation/map_batch.cpp
    src/map_generation/map_storage.cpp
    src/map_generation/map_generator_cli.cpp
    src/project2_session.cpp
    src/project2_application.cpp
)
# Include headers
target_include_directories(Maze_core PUBLIC include)

target_link_libraries(Maze_core PUBLIC
        ftxui::screen
        ftxui::dom
        ftxui::component
)

# Create the executable game
add_executable(Maze
        app/main.cpp
        include/circuit_escape/graph.h
)

target_link_libraries(Maze PRIVATE
        Maze_core
)

add_executable(map_generator
    app/map_generator_main.cpp
)
target_link_libraries(map_generator PRIVATE Maze_core)

add_executable(escape_circuit app/escape_circuit_main.cpp)
target_link_libraries(escape_circuit PRIVATE Maze_core)

#Tests

enable_testing()

add_executable(Maze_test
    tests/test_main.cpp
    tests/grid_test.cpp
    tests/types_test.cpp
    tests/agent_test.cpp
    tests/environment_test.cpp
    tests/interactions_test.cpp
    tests/controllers_test.cpp
    tests/simulation_test.cpp
    tests/scenario_1_test.cpp
    tests/scenario_2_test.cpp
    tests/generic_functions_test.cpp
    tests/console_ui_test.cpp
    tests/event_test.cpp
    tests/console_render_test.cpp
    tests/map_generation/map_selector_test.cpp
    tests/help_test.cpp
    tests/menus_test.cpp
    tests/animations_test.cpp
        tests/graph_test.cpp
    tests/graph/disjoint_set_test.cpp
    tests/map_generation/kruskal_test.cpp
    tests/pathfinding/dijkstra_test.cpp
    tests/map_generation/map_validator_test.cpp
    tests/map_generation/game_verifier_test.cpp
    tests/map_generation/map_batch_test.cpp
    tests/map_generation/map_storage_test.cpp
    tests/map_generation/map_generator_cli_test.cpp
    tests/project2_integration_test.cpp
)

target_link_libraries(Maze_test PRIVATE Maze_core)

add_test(NAME Maze_test COMMAND Maze_test)

# Runtime distribution for Windows installer.
install(TARGETS map_generator escape_circuit
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)
install(FILES README.md DESTINATION . COMPONENT Runtime)
install(FILES
    installer/launch-game.cmd
    installer/launch-generator.cmd
    DESTINATION .
    COMPONENT Runtime
)
