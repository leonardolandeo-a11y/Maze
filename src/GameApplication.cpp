#include "circuit_escape/GameApplication.h"

#include <array>
#include <string>
#include <atomic>
#include <chrono>
#include <thread>

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

void GameApplication::RunStartupAnimation(){
    using namespace ftxui;
    using namespace std::chrono_literals;

    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    int frame = 0;

    constexpr int bootEndFrame = 38;
    constexpr int scanStartFrame = 42;
    constexpr int maxFrames = 67;

    std::atomic<bool> animationRunning{true};

    Component renderer = Renderer([&] {
        //Secuencia inicial del sistema
        if (frame < bootEndFrame) {
            Elements bootLines;

            if (frame >= 4) {
                bootLines.push_back(text("> INITIALIZING CORE...") | color(Color::GreenLight));
            }

            if (frame >= 9) {
                bootLines.push_back(text("> NAVIGATION SYSTEM ........ OK") | color(Color::GreenLight));
            }

            if (frame >= 14) {
                bootLines.push_back(text("> GRID MATRIX .............. OK") | color(Color::GreenLight));
            }

            if (frame >= 19) {
                bootLines.push_back(text("> MEMORY CHECK ............. OK") | color(Color::GreenLight));
            }

            if (frame >= 24) {
                bootLines.push_back(text("> ESCAPE PROTOCOL .......... ACTIVE") | color(Color::CyanLight));
            }

            if (frame >= 29) {
                bootLines.push_back(text(""));
                bootLines.push_back(text("> SYSTEM LINK ESTABLISHED") | color(Color::CyanLight) | bold);
            }

            if (frame >= 4) {
                const std::string cursor = frame % 2 == 0 ? "█" : " ";
                bootLines.push_back(text(cursor) | color(Color::GreenLight));
            }

            return vbox({
                filler(),
                hbox({
                    filler(),
                    vbox(bootLines),
                    filler()
                }),
                filler()
            }) | bgcolor(Color::Black);
        }

        //Pequena pausa negra entre el inicio y el escaneo
        if (frame < scanStartFrame) {
            return vbox({
                filler(),
                text(""),
                filler()
            }) | bgcolor(Color::Black);
        }

        //Escaneo digital del laberinto
        const int scanFrame = frame - scanStartFrame;

        const std::array<std::string, 9> mazeLines = {
            "########################################",
            "# @        #              #            #",
            "# #######  #  ##########  #  #######   #",
            "#       #     #              #         #",
            "######  #######  ##########  #  #####  #",
            "#       #        #           #         #",
            "#  ###########   #   ###############   #",
            "#                             EXIT     #",
            "########################################"
        };

        const std::array<std::string, 6> noiseFrames = {
            "01#@:://001101##::0101",
            "##10::SIGNAL//001::#@",
            "0110##:://GRID::10110",
            "@@01::MATRIX//##00110",
            "10##::SYNC//0101::@@",
            "001101:://##::SYSTEM"
        };

        Elements scanLines;

        scanLines.push_back(
            text(">>> GRID MATRIX SCAN") |
            color(Color::CyanLight) |
            bold
        );

        scanLines.push_back(text(""));

        const int visibleRows = scanFrame / 2;

        for (std::size_t i = 0; i < mazeLines.size(); ++i) {
            if (static_cast<int>(i) < visibleRows) {
                scanLines.push_back(
                    text(mazeLines[i]) |
                    color(Color::GreenLight)
                );
            }
            else if (static_cast<int>(i) == visibleRows && visibleRows < static_cast<int>(mazeLines.size())) {
                scanLines.push_back(
                    text(mazeLines[i]) |
                    color(Color::CyanLight) |
                    bold
                );
            }
            else {
                scanLines.push_back(text(""));
            }
        }

        scanLines.push_back(text(""));

        int scanProgress = (scanFrame * 100) / 22;

        if (scanProgress > 100) {
            scanProgress = 100;
        }

        scanLines.push_back(
            text("SCAN PROGRESS: " + std::to_string(scanProgress) + "%") |
            color(Color::CyanLight)
        );

        scanLines.push_back(
            text(noiseFrames[scanFrame % noiseFrames.size()]) |
            color(Color::GreenLight)
        );

        if (scanFrame >= 20) {
            scanLines.push_back(text(""));

            scanLines.push_back(
                text("> GRID SIGNAL ACQUIRED") |
                color(Color::CyanLight) |
                bold
            );
        }

        return vbox({
            filler(),
            hbox({
                filler(),
                vbox(scanLines),
                filler()
            }),
            filler()
        }) | bgcolor(Color::Black);
    });

    std::thread animationThread([&] {
        while (animationRunning) {
            std::this_thread::sleep_for(110ms);

            if (!animationRunning) {
                break;
            }

            screen.Post([&] {
                ++frame;

                if (frame >= maxFrames) {
                    animationRunning = false;
                    screen.Exit();
                }
            });

            screen.Post(Event::Custom);
        }
    });

    screen.Loop(renderer);

    animationRunning = false;

    if (animationThread.joinable()) {
        animationThread.join();
    }
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
    RunStartupAnimation();

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

