#include <cassert>

#include "circuit_escape/console_ui.h"


void test_key_mapping_lowercase() {
    ConsoleUI ui;

    assert(ui.KeyMapping(ftxui::Event::Character('w'))== UICommand::up);
    assert(ui.KeyMapping(ftxui::Event::Character('s'))== UICommand::down);
    assert(ui.KeyMapping(ftxui::Event::Character('a'))== UICommand::left);
    assert(ui.KeyMapping(ftxui::Event::Character('d'))== UICommand::right);
    assert(ui.KeyMapping(ftxui::Event::Character('e'))== UICommand::wait);
    assert(ui.KeyMapping(ftxui::Event::Character('h'))== UICommand::help);
    assert(ui.KeyMapping(ftxui::Event::Character('q'))== UICommand::quit);
}


void test_key_mapping_uppercase() {
    ConsoleUI ui;

    assert(ui.KeyMapping(ftxui::Event::Character('W'))== UICommand::up);
    assert(ui.KeyMapping(ftxui::Event::Character('S'))== UICommand::down);
    assert(ui.KeyMapping(ftxui::Event::Character('A'))== UICommand::left);
    assert(ui.KeyMapping(ftxui::Event::Character('D'))== UICommand::right);
    assert(ui.KeyMapping(ftxui::Event::Character('E'))== UICommand::wait);
    assert(ui.KeyMapping(ftxui::Event::Character('H'))== UICommand::help);
    assert(ui.KeyMapping(ftxui::Event::Character('Q'))== UICommand::quit);
}


void test_unknown_key_returns_nullopt() {
    ConsoleUI ui;

    const auto command =
        ui.KeyMapping(
            ftxui::Event::Character('x')
        );

    assert(!command.has_value());
}

void test_unknown_key_does_not_modify_environment() {
    ConsoleUI ui;

    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = rulesFor(Difficulty::standard);

    Agent agent(
        {0, 0},
        rules.initialEnergy,
        rules.maximumEnergy
    );

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    const Observation before = environment.state();

    const auto command =
        ui.KeyMapping(
            ftxui::Event::Character('x')
        );

    assert(!command.has_value());

    const Observation after = environment.state();

    assert(after.agent == before.agent);
    assert(after.goal == before.goal);
    assert(after.energy == before.energy);
    assert(after.maximumEnergy == before.maximumEnergy);
    assert(after.score == before.score);
    assert(after.collectedResources == before.collectedResources);
    assert(after.turn == before.turn);
    assert(after.turnLimit == before.turnLimit);
    assert(after.availableActions == before.availableActions);
}

void run_console_ui_tests() {
    test_key_mapping_lowercase();
    test_key_mapping_uppercase();
    test_unknown_key_returns_nullopt();
    test_unknown_key_does_not_modify_environment();
}