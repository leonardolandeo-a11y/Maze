#pragma once
#include "circuit_escape/environment.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include "circuit_escape/agent.h"
#include "circuit_escape/console_ui.h"
class GameApplication{
private:
    Grid<Cell, 20,30> CreateScenario();
    Agent CreatePlayer();
    ftxui::Component CreateGameComponent(NavigationEnvironment<20,30>& environment,ConsoleUI& ui, std::vector<NavigationEvent>& recentEvents, ftxui::ScreenInteractive& screen);
    
public:
    void Run();
};