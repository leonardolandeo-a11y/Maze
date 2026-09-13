#include "circuit_escape/GameApplication.h"
#include "circuit_escape/scenarios/scenario_1.h"

Grid<Cell, 20,30> GameApplication::CreateScenario(){
    return createScenario1();
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
            if (command == UICommand::wait){
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
