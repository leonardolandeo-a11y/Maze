#include "circuit_escape/environment.h"
#include "circuit_escape/console_ui.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include "circuit_escape/game_rules.h"
#include <vector>
#include <iostream>


int main(){
    Grid<Cell, 20,30> grid;
    grid.at({0,3}) = Wall{};
    grid.at({0,4}) = Battery{};
    grid.at({19,29}) = Exit{};

    Agent agent({0,0}, 10, 10);
    GameRules rules = rulesFor(Difficulty::hard);

    NavigationEnvironment<20,30> environment(grid, agent, rules);

    ConsoleUI ui (RenderMode::emoji);

    std::vector<NavigationEvent> events;
    
    auto document = ui.render(environment, events);

    auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(document));

    ftxui::Render(screen, document);
    std::cout<<screen.ToString();
    return 0;
}
