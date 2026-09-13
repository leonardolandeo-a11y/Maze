#include "circuit_escape/GameApplication.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "circuit_escape/console_ui.h"
#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/DeathAnimation.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

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

            if (!command.has_value()){
                return false;
            }

            if (command == UICommand::quit){
                screen.Exit();
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

void GameApplication::Run(){
    auto grid = CreateScenario();
    Agent player = CreatePlayer();
    GameRules rules = rulesFor(Difficulty::standard);

    NavigationEnvironment<20,30> environment(grid,player,rules);

    ConsoleUI ui(RenderMode::emoji);

    std::vector<NavigationEvent> recentEvents;

    ftxui::ScreenInteractive screen =
        ftxui::ScreenInteractive::Fullscreen();

    std::atomic<bool> startupActive{true};
    std::atomic<bool> deathActive{false};
    std::atomic<bool> deathAnimationComplete{false};
    std::atomic<bool> applicationRunning{true};

    StartupAnimation startupAnimation;
    DeathAnimation deathAnimation;

    int startupFrame = 0;
    int deathFrame = 0;

    EndReason deathReason = EndReason::none;

    auto handleStepResult = [&](const StepResult& result){
        if (!result.finished){
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

            if (deathActive){
                return deathAnimation.RenderDeathFrame(
                    deathFrame,
                    deathReason
                );
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

                        if (
                            startupFrame >=
                            StartupAnimation::maxFrames
                        ){
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

                        if (
                            deathFrame >=
                            DeathAnimation::maxFrames
                        ){
                            deathAnimationComplete = true;
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

    if (animationThread.joinable()){
        animationThread.join();
    }
}