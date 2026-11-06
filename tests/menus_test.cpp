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
    assert(menu.takeCommand() == MenuCommand::none);
    choose(menu);
    assert(menu.takeCommand() == MenuCommand::generateMaps);
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

void test_project2_size_flow() {
    assert(project2_map_sizes.size() == 3);
    for (int index = 0; index < 3; ++index) {
        Menus menu(true);
        choose(menu);
        move_down(menu, index);
        choose(menu);
        assert(menu.selectedMapSize() == project2_map_sizes[index]);
        assert(menu.takeCommand() == MenuCommand::generateMaps);
        assert(menu.takeCommand() == MenuCommand::none);
        menu.generationFinished(true);
        move_down(menu, index);
        choose(menu);
        assert(menu.takeCommand() == MenuCommand::startGame);
        assert(menu.selectedDifficulty() == (index == 0 ? Difficulty::easy :
            index == 1 ? Difficulty::standard : Difficulty::hard));
        menu.generationFinished(false);
        choose(menu); // Main -> sizes, never gameplay after failure.
        assert(menu.takeCommand() == MenuCommand::none);
        move_down(menu, 4);
        choose(menu); // Back.
        assert(menu.takeCommand() == MenuCommand::none);
    }
}

void test_custom_sizes() {
    for (const auto size : {Project2MapSize{30,40}, Project2MapSize{15,20},
        Project2MapSize{100,150}, Project2MapSize{10,60}, Project2MapSize{40,10}}) {
        Menus menu(true);
        choose(menu);
        move_down(menu, 3);
        choose(menu);
        menu.OnEvent(ftxui::Event::Character(std::to_string(size.rows)));
        menu.OnEvent(ftxui::Event::Character('x')); // letters never enter the field
        menu.OnEvent(ftxui::Event::Tab);
        menu.OnEvent(ftxui::Event::Character(std::to_string(size.columns)));
        choose(menu); // field -> generate
        choose(menu);
        const bool valid = size == Project2MapSize{30,40} || size == Project2MapSize{15,20};
        assert(menu.takeCommand() == (valid ? MenuCommand::generateMaps : MenuCommand::none));
        if (valid) assert(menu.selectedMapSize() == size);
        assert(menu.Render());
    }
    Menus menu(true);
    choose(menu); move_down(menu, 3); choose(menu);
    menu.OnEvent(ftxui::Event::Backspace); // empty rows
    choose(menu); choose(menu); choose(menu);
    assert(menu.takeCommand() == MenuCommand::none);
    menu.OnEvent(ftxui::Event::Escape);
    choose(menu); // Custom again
    menu.OnEvent(ftxui::Event::ArrowUp); // Back
    choose(menu);
    assert(menu.takeCommand() == MenuCommand::none);
}

void test_project2_responsive_layout_policy() {
    assert(project2_menu_layout_for(160, 45) == Project2MenuLayout::split);
    assert(project2_menu_layout_for(120, 30) == Project2MenuLayout::split);
    assert(project2_menu_layout_for(80, 24) == Project2MenuLayout::stacked);
    assert(project2_menu_layout_for(70, 20) == Project2MenuLayout::stacked);
    assert(project2_menu_layout_for(47, 30) == Project2MenuLayout::compact);
    assert(project2_menu_layout_for(100, 17) == Project2MenuLayout::compact);
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
    test_project2_size_flow();
    test_custom_sizes();
    test_project2_responsive_layout_policy();
    test_project1_compatibility();
}
