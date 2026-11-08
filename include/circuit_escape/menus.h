#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include "circuit_escape/game_rules.h"
#include "circuit_escape/project2_generation.h"
#include "circuit_escape/console_ui.h"

enum class MenuScreen {
    Main,
    Scenario,
    MapSize,
    CustomSize,
    Difficulty,
    RenderMode,
    Help
};

enum class MenuCommand {
    none,
    startGame,
    generateMaps,
    quit
};

enum class ScenarioSelection {
    scenario1,
    scenario2
};

enum class Project2MenuLayout {
    compact,
    stacked,
    split
};

[[nodiscard]] Project2MenuLayout project2_menu_layout_for(int width, int height) noexcept;

class Menus {
public:
    explicit Menus(bool project2_mode = false);
    [[nodiscard]] ftxui::Element Render(std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
    bool OnEvent(ftxui::Event event);
    MenuCommand takeCommand();
    [[nodiscard]]ScenarioSelection selectedScenario() const noexcept;
    [[nodiscard]]Difficulty selectedDifficulty() const noexcept;
    [[nodiscard]]RenderMode selectedRenderMode() const noexcept;

    Project2MapSize selectedMapSize() const { return selectedMapSize_; }
    void generationFinished(bool success);

private:
    Project2MapSize selectedMapSize_ = project2_map_sizes.front();
    std::string customRows_ = "20";
    std::string customColumns_ = "30";
    bool replaceInput_ = true;
    bool mapsReady_ = false;
    bool project2_mode_ = false;
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