#include "circuit_escape/GameApplication.h"

Grid<Cell, 20,30> GameApplication::CreateScenario(){
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
    return grid;
}

Agent GameApplication::CreatePlayer(){
    return Agent({1,1}, 10,10);
}


ftxui::Component GameApplication::CreateGameComponent(NavigationEnvironment<20,30>& environment,ConsoleUI& ui, std::vector<NavigationEvent>& recentEvents, ftxui::ScreenInteractive& screen){
    ftxui::Component renderer = ftxui::Renderer(
        [&]{
            return ui.render(environment, recentEvents);
    });

    ftxui::Component component = ftxui::CatchEvent(renderer,
        [&](const ftxui::Event& event){
            std::optional<UICommand> command = ui.KeyMapping(event);

            if (!command.has_value()){
                return false;
            }
            if (command == UICommand::up){
                StepResult result = environment.step(Action::up);
                recentEvents = result.events;
            }
            if (command == UICommand::down){
                StepResult result = environment.step(Action::down);
                recentEvents = result.events;
            }
            if (command == UICommand::right){
                StepResult result = environment.step(Action::right);
                recentEvents = result.events;
            }
            if (command == UICommand::left){
                StepResult result = environment.step(Action::left);
                recentEvents = result.events;
            }
            if (command== UICommand::wait){
                StepResult result = environment.step(Action::wait);
                recentEvents = result.events;
            }
            if (command == UICommand::quit){
                screen.Exit();
                return true;
            }
            return true;
        }
    );
    return component;
}


void GameApplication::Run(){
    auto grid = CreateScenario();
    Agent player = CreatePlayer();
    GameRules rules = rulesFor(Difficulty::standard);
    NavigationEnvironment<20,30> environment(grid, player,rules);
    ConsoleUI ui(RenderMode::emoji);
    std::vector<NavigationEvent> recentEvents;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
    ftxui::Component component = CreateGameComponent(environment,ui,recentEvents,screen);
    screen.Loop(component);


}

