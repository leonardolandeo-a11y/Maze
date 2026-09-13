#include "circuit_escape/environment.h"
#include "circuit_escape/console_ui.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include "circuit_escape/game_rules.h"
#include "ftxui/component/screen_interactive.hpp"
#include <ftxui/component/component.hpp>

#include <vector>
#include <iostream>


int main(){
    Grid<Cell, 20,30> grid;

    // -------------------------
    // Temporary test scenario
    // -------------------------

    // Walls
    grid.at({0, 3}) = Wall{};
    grid.at({0, 4}) = Wall{};
    grid.at({0, 5}) = Wall{};

    grid.at({1, 5}) = Wall{};
    grid.at({2, 5}) = Wall{};
    grid.at({3, 5}) = Wall{};

    // Rough terrain
    grid.at({2, 2}) = RoughTerrain{};
    grid.at({2, 3}) = RoughTerrain{};
    grid.at({3, 2}) = RoughTerrain{};

    // Resources
    grid.at({4, 4}) = ResourceCell<int>{10};
    grid.at({6, 8}) = ResourceCell<int>{20};

    // Batteries
    grid.at({1, 8}) = Battery{};
    grid.at({8, 12}) = Battery{};

    // Traps
    grid.at({5, 5}) = Trap{};
    grid.at({7, 10}) = Trap{};

    // Exit
    grid.at({10, 15}) = Exit{};

    // Agent starts here
    Agent agent({1, 1}, 10, 10);

    GameRules rules = rulesFor(Difficulty::hard);

    NavigationEnvironment<20,30> environment(grid, agent, rules);


    ConsoleUI ui(RenderMode::emoji);

    std::vector<NavigationEvent> events;

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    auto rendered = ftxui::Renderer(
        [&] {return ui.render(environment,events);}
    );
    auto component = ftxui::CatchEvent(rendered, 
    [&](ftxui::Event event){
        auto command = ui.KeyMapping(event);
        if (!command.has_value()){
            return false;
        }
        switch (command.value()){
        
        case UICommand::up:{
            StepResult result = environment.step(Action::up);
            events = result.events;
            break;
        }
        default:
            break;
        }
        return true;
    });
    screen.Loop(component);

    return 0;
}
