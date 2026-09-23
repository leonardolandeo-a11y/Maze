#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include "circuit_escape/game_rules.h"
#include "circuit_escape/console_ui.h"

enum class MenuScreen {
    Main,
    Scenario,
    Difficulty,
    RenderMode,
    Help
};

enum class MenuCommand {
    none,
    startGame,
    quit
};

enum class ScenarioSelection {
    scenario1,
    scenario2
};

class Menus {
public:
    [[nodiscard]] ftxui::Element Render() const;
    bool OnEvent(ftxui::Event event);
    MenuCommand takeCommand();
    [[nodiscard]]ScenarioSelection selectedScenario() const noexcept;
    [[nodiscard]]Difficulty selectedDifficulty() const noexcept;
    [[nodiscard]]RenderMode selectedRenderMode() const noexcept;

private:
    MenuScreen currentScreen_ = MenuScreen::Main;
    int selectedOption_ = 0;
    /*
    Atributos agregados para poder: 
    - cambiar de menu
    -cambiar escenario
    - dificultad
    - renderizacion
    */
    MenuCommand pendingCommand_ = MenuCommand::none;
    ScenarioSelection selectedScenario_ = ScenarioSelection::scenario1;
    Difficulty selectedDifficulty_ = Difficulty::standard;
    RenderMode selectedRenderMode_ = RenderMode::emoji;
};