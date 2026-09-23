#pragma once

#include "circuit_escape/environment.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/agent.h"
#include "circuit_escape/console_ui.h"
#include "circuit_escape/menus.h"

#include <functional>

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component.hpp>

class GameApplication{
private:
    Grid<Cell, 20,30> CreateScenario(ScenarioSelection scenario);
    Agent CreatePlayer(const GameRules& rules);

    ftxui::Component CreateGameComponent(
        NavigationEnvironment<20,30>& environment,
        ConsoleUI& ui,
        std::vector<NavigationEvent>& recentEvents,
        ftxui::ScreenInteractive& screen,
        std::function<void(const StepResult&)> onStepResult
    );

public:
    void Run();
};