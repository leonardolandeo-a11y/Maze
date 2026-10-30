#include "circuit_escape/menus.h"

#include <cassert>

namespace {

void move_down(Menus& menu, int steps) {
    for (int index = 0; index < steps; ++index) {
        assert(menu.OnEvent(ftxui::Event::ArrowDown));
    }
}

void choose(Menus& menu) {
    assert(menu.OnEvent(ftxui::Event::Return));
}

void test_project2_defaults_and_commands() {
    Menus menu(true);
    assert(menu.selectedDifficulty() == Difficulty::easy);
    assert(menu.selectedRenderMode() == RenderMode::emoji);
    assert(menu.takeCommand() == MenuCommand::none);
    assert(!menu.OnEvent(ftxui::Event::Character('x')));

    choose(menu);
    assert(menu.takeCommand() == MenuCommand::startGame);
    assert(menu.takeCommand() == MenuCommand::none);

    Menus quit_menu(true);
    assert(quit_menu.OnEvent(ftxui::Event::ArrowUp));
    choose(quit_menu);
    assert(quit_menu.takeCommand() == MenuCommand::quit);
    assert(quit_menu.takeCommand() == MenuCommand::none);
}

void test_project2_difficulty_navigation() {
    Menus menu(true);

    move_down(menu, 1);
    choose(menu); // Difficulty screen.
    move_down(menu, 2);
    choose(menu); // Hard.
    assert(menu.selectedDifficulty() == Difficulty::hard);
    assert(menu.takeCommand() == MenuCommand::none);

    move_down(menu, 1);
    choose(menu); // Difficulty screen again.
    move_down(menu, 3);
    choose(menu); // Back without changing selection.
    assert(menu.selectedDifficulty() == Difficulty::hard);

    move_down(menu, 1);
    choose(menu);
    assert(menu.OnEvent(ftxui::Event::Character('s')));
    choose(menu); // Medium.
    assert(menu.selectedDifficulty() == Difficulty::standard);

    move_down(menu, 1);
    choose(menu);
    choose(menu); // Easy.
    assert(menu.selectedDifficulty() == Difficulty::easy);
}

void test_project2_render_mode_and_help() {
    Menus menu(true);

    move_down(menu, 2);
    choose(menu); // Render mode.
    assert(menu.OnEvent(ftxui::Event::Character('S')));
    choose(menu); // ASCII.
    assert(menu.selectedRenderMode() == RenderMode::ascii);

    move_down(menu, 2);
    choose(menu);
    move_down(menu, 2);
    choose(menu); // Back.
    assert(menu.selectedRenderMode() == RenderMode::ascii);

    move_down(menu, 3);
    choose(menu); // Help.
    choose(menu); // Back to main.
    assert(menu.takeCommand() == MenuCommand::none);

    move_down(menu, 2);
    choose(menu); // Render mode.
    choose(menu); // Emoji.
    assert(menu.selectedRenderMode() == RenderMode::emoji);
}

void test_project1_compatibility() {
    Menus menu(false);
    assert(menu.selectedScenario() == ScenarioSelection::scenario1);
    assert(menu.selectedDifficulty() == Difficulty::standard);
    assert(menu.selectedRenderMode() == RenderMode::emoji);

    move_down(menu, 1);
    choose(menu); // Scenario screen.
    move_down(menu, 1);
    choose(menu); // Scenario 2.
    assert(menu.selectedScenario() == ScenarioSelection::scenario2);

    move_down(menu, 1);
    choose(menu);
    move_down(menu, 2);
    choose(menu); // Back, keeping Scenario 2.
    assert(menu.selectedScenario() == ScenarioSelection::scenario2);

    move_down(menu, 2);
    choose(menu); // Difficulty screen.
    move_down(menu, 2);
    choose(menu); // Hard.
    assert(menu.selectedDifficulty() == Difficulty::hard);

    move_down(menu, 4);
    choose(menu); // Help screen.
    choose(menu); // Back.
    assert(menu.takeCommand() == MenuCommand::none);

    Menus quit_menu(false);
    assert(quit_menu.OnEvent(ftxui::Event::Character('w')));
    choose(quit_menu);
    assert(quit_menu.takeCommand() == MenuCommand::quit);
    assert(quit_menu.takeCommand() == MenuCommand::none);
}

} // namespace

void run_menus_tests() {
    test_project2_defaults_and_commands();
    test_project2_difficulty_navigation();
    test_project2_render_mode_and_help();
    test_project1_compatibility();
}
