#include "circuit_escape/GameApplication.h"
#include "circuit_escape/scenarios/scenario_1.h"
#include "circuit_escape/scenarios/scenario_2.h"


#include <atomic>
#include <chrono>
#include <optional>
#include <thread>
#include <vector>

#include "circuit_escape/console_ui.h"
#include "circuit_escape/menus.h"
#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/DeathAnimation.h"
#include "circuit_escape/VictoryAnimation.h"

#include <ftxui/dom/elements.hpp>

Grid<Cell, 20,30> GameApplication::CreateScenario(ScenarioSelection scenario){
    switch (scenario) {

        case ScenarioSelection::scenario1:
            return createScenario1();
        case ScenarioSelection::scenario2:
            return createScenario2();
    }

    return createScenario1();
}


Agent GameApplication::CreatePlayer(const GameRules& rules) {
    return Agent(
        {1, 1},
        rules.initialEnergy,
        rules.maximumEnergy
    );
}

/*---------------------------------------------------------------------
CreateGameComponent() -> Construye el componente interactivo de la partida.

ConsoleUI traduce el evento de teclado a UICommand. Las acciones
jugables se envían a NavigationEnvironment::step(), mientras que
los comandos de ayuda y salida se resuelven en la capa de aplicación.

El StepResult generado actualiza los eventos visibles y permite
activar las pantallas de victoria o derrota.
---------------------------------------------------------------------*/
ftxui::Component GameApplication::CreateGameComponent(
    NavigationEnvironment<20,30>& environment,
    ConsoleUI& ui,
    std::vector<NavigationEvent>& recentEvents,
    ftxui::ScreenInteractive& screen,
    std::function<void(const StepResult&)> onStepResult
){
    ftxui::Component renderer = ftxui::Renderer(
        [&]{
            return ftxui::vbox({
                ftxui::filler(),
                ftxui::hbox({
                    ftxui::filler(),
                    ui.render(environment, recentEvents),
                    ftxui::filler()
                }),
                ftxui::filler()
            });
        }
    );

    ftxui::Component component = ftxui::CatchEvent(
        renderer,
        [&, onStepResult](const ftxui::Event& event){
            std::optional<UICommand> command = ui.KeyMapping(event);

            if (!command.has_value()) {
                if (event.is_character()) {
                    ui.setInputMessage("Unknown command");
                    return true;
                }

                return false;
            }
            ui.clearInputMessage();

            if (command == UICommand::quit){
                screen.Exit();
                return true;
            }
            if (command == UICommand::help) {
                ui.setHelp();
                return true;
            }
            if (ui.isHelpVisible()) {
                return true;
            }
            if (environment.isFinished()){
                return true;
            }

            if (command == UICommand::up){
                StepResult result = environment.step(Action::up);
                recentEvents = result.events;
                onStepResult(result);
            }

            if (command == UICommand::down){
                StepResult result = environment.step(Action::down);
                recentEvents = result.events;
                onStepResult(result);
            }

            if (command == UICommand::right){
                StepResult result = environment.step(Action::right);
                recentEvents = result.events;
                onStepResult(result);
            }

            if (command == UICommand::left){
                StepResult result = environment.step(Action::left);
                recentEvents = result.events;
                onStepResult(result);
            }

            if (command == UICommand::wait){
                StepResult result = environment.step(Action::wait);
                recentEvents = result.events;
                onStepResult(result);
            }

            return true;
        }
    );

    return component;
}
/*---------------------------------------------------------------------
Run() -> Coordina el ciclo completo de la aplicación.

El renderer selecciona la vista activa entre animación inicial,
menú, partida, victoria y derrota. Los eventos del menú configuran
escenario, dificultad y modo de render antes de crear el entorno.
---------------------------------------------------------------------*/
void GameApplication::Run(){
    bool playAgain = true;
    bool firstGame = true;

    while (playAgain){
        playAgain = false;

        auto grid = CreateScenario(ScenarioSelection::scenario1);
        GameRules rules = rulesFor(Difficulty::standard);
        Agent player = CreatePlayer(rules);

        NavigationEnvironment<20,30> environment(grid,player,rules);
        ConsoleUI ui(RenderMode::emoji);

        std::vector<NavigationEvent> recentEvents;

        ftxui::ScreenInteractive screen =
            ftxui::ScreenInteractive::Fullscreen();

        std::atomic<bool> startupActive{firstGame};
        std::atomic<bool> menuActive{true};
        std::atomic<bool> deathActive{false};
        std::atomic<bool> deathAnimationComplete{false};
        std::atomic<bool> victoryActive{false};
        std::atomic<bool> victoryAnimationComplete{false};
        std::atomic<bool> applicationRunning{true};

        firstGame = false;

        bool restartRequested = false;

        StartupAnimation startupAnimation;
        DeathAnimation deathAnimation;
        VictoryAnimation victoryAnimation;
        Menus menus;

        int startupFrame = 0;
        int deathFrame = 0;
        int victoryFrame = 0;

        EndReason deathReason = EndReason::none;
        std::optional<Observation> finalObservation;

        auto handleStepResult = [&](const StepResult& result){
            if (!result.finished){
                return;
            }
            finalObservation = result.observation;
            if (result.reason == EndReason::goalReached){
                victoryFrame = 0;
                victoryAnimationComplete = false;
                victoryActive = true;
                return;
            }

            if (
                result.reason == EndReason::noEnergy ||
                result.reason == EndReason::turnLimit
            ){
                deathReason = result.reason;
                deathFrame = 0;
                deathAnimationComplete = false;
                deathActive = true;
            }
        };

        ftxui::Component gameComponent = CreateGameComponent(
            environment,
            ui,
            recentEvents,
            screen,
            handleStepResult
        );

        ftxui::Component applicationRenderer = ftxui::Renderer(
            [&]{
                if (startupActive){
                    return startupAnimation.RenderStartupFrame(startupFrame);
                }

                if (menuActive){
                    return menus.Render();
                }

                if (deathActive){
                    return deathAnimation.RenderDeathFrame(deathFrame,deathReason,*finalObservation);
                }

                if (victoryActive){
                    return victoryAnimation.RenderVictoryFrame(victoryFrame,*finalObservation);
                }

                return gameComponent->Render();
            }
        );

        ftxui::Component application = ftxui::CatchEvent(
            applicationRenderer,
            [&](const ftxui::Event& event){
                if (startupActive){
                    if (event == ftxui::Event::Return){
                        startupActive = false;
                        return true;
                    }

                    return true;
                }

                if (menuActive) {
                    bool eventhandled = menus.OnEvent(event);
                    MenuCommand command = menus.takeCommand();
                    switch (command) {
                        case MenuCommand::startGame:
                            grid = CreateScenario( menus.selectedScenario() );
                            rules = rulesFor(menus.selectedDifficulty());
                            player = CreatePlayer(rules);
                            environment = NavigationEnvironment<20, 30>(
                                            grid,
                                            player,
                                            rules
                                        );
                            ui = ConsoleUI(menus.selectedRenderMode());
                            recentEvents.clear();
                            menuActive = false;
                            return true;
                        case MenuCommand::quit :
                            screen.Exit();
                            return true;
                        case MenuCommand::none:
                            break;
                    }
                    return eventhandled;
                }
                
                if (deathActive){
                    if (
                        event == ftxui::Event::Character('q') ||
                        event == ftxui::Event::Character('Q')
                    ){
                        screen.Exit();
                        return true;
                    }

                    if (
                        deathAnimationComplete &&
                        event == ftxui::Event::Return
                    ){
                        restartRequested = true;
                        screen.Exit();
                        return true;
                    }

                    return true;
                }

                if (victoryActive){
                    if (
                        event == ftxui::Event::Character('q') ||
                        event == ftxui::Event::Character('Q')
                    ){
                        screen.Exit();
                        return true;
                    }

                    if (
                        victoryAnimationComplete &&
                        event == ftxui::Event::Return
                    ){
                        restartRequested = true;
                        screen.Exit();
                        return true;
                    }

                    return true;
                }

                return gameComponent->OnEvent(event);
            }
        );

        std::thread animationThread(
            [&]{
                while (applicationRunning){
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(60)
                    );

                    if (!applicationRunning){
                        break;
                    }

                    if (startupActive){
                        screen.Post([&]{
                            ++startupFrame;

                            if (startupFrame >= StartupAnimation::maxFrames){
                                startupActive = false;
                            }
                        });

                        screen.Post(ftxui::Event::Custom);
                        continue;
                    }

                    if (
                        deathActive &&
                        !deathAnimationComplete
                    ){
                        screen.Post([&]{
                            ++deathFrame;

                            if (deathFrame >= DeathAnimation::maxFrames){
                                deathAnimationComplete = true;
                            }
                        });

                        screen.Post(ftxui::Event::Custom);
                        continue;
                    }

                    if (
                        victoryActive &&
                        !victoryAnimationComplete
                    ){
                        screen.Post([&]{
                            ++victoryFrame;

                            if (victoryFrame >= VictoryAnimation::maxFrames){
                                victoryAnimationComplete = true;
                            }
                        });

                        screen.Post(ftxui::Event::Custom);
                    }
                }
            }
        );

        screen.Loop(application);

        applicationRunning = false;
        startupActive = false;
        deathActive = false;
        victoryActive = false;

        if (animationThread.joinable()){
            animationThread.join();
        }

        if (restartRequested){
            playAgain = true;
        }
    }
}
