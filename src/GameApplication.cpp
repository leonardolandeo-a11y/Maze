#include "circuit_escape/GameApplication.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

namespace {
    constexpr int gameViewWidth = 90;
    constexpr int gameViewHeight = 20;

    constexpr int bootEndFrame = 38;
    constexpr int glitchStartFrame = 42;

    constexpr int logoFormationStartFrame = 64;
    constexpr int titleRevealFrame = 94;

    constexpr int subtitleFrame = 110;

    constexpr int loadingStartFrame = 124;
    constexpr int loadingCompleteFrame = 156;

    constexpr int maxFrames = 166;

    ftxui::Element CenterGameView(ftxui::Element content){
        using namespace ftxui;

        Element fixedView =
            content |
            size(WIDTH, EQUAL, gameViewWidth) |
            size(HEIGHT, EQUAL, gameViewHeight);

        return vbox({
            filler(),
            hbox({
                filler(),
                fixedView,
                filler()
            }),
            filler()
        }) | bgcolor(Color::Black);
    }

    ftxui::Element RenderStartupFrame(int frame){
        using namespace ftxui;

        //Secuencia inicial tipo sistema
        if (frame < bootEndFrame) {
            Elements bootLines;

            if (frame >= 4) {
                bootLines.push_back(
                    text("> INITIALIZING CORE...") |
                    color(Color::GreenLight)
                );
            }

            if (frame >= 9) {
                bootLines.push_back(
                    text("> NAVIGATION SYSTEM ........ OK") |
                    color(Color::GreenLight)
                );
            }

            if (frame >= 14) {
                bootLines.push_back(
                    text("> GRID MATRIX .............. OK") |
                    color(Color::GreenLight)
                );
            }

            if (frame >= 19) {
                bootLines.push_back(
                    text("> MEMORY CHECK ............. OK") |
                    color(Color::GreenLight)
                );
            }

            if (frame >= 24) {
                bootLines.push_back(
                    text("> ESCAPE PROTOCOL .......... ACTIVE") |
                    color(Color::CyanLight)
                );
            }

            if (frame >= 29) {
                bootLines.push_back(text(""));

                bootLines.push_back(
                    text("> SYSTEM LINK ESTABLISHED") |
                    color(Color::CyanLight) |
                    bold
                );
            }

            if (frame >= 4) {
                const std::string cursor = frame % 2 == 0 ? "█" : " ";

                bootLines.push_back(
                    text(cursor) |
                    color(Color::GreenLight)
                );
            }

            return CenterGameView(
                vbox({
                    filler(),
                    hbox({
                        filler(),
                        vbox(bootLines),
                        filler()
                    }),
                    filler()
                }) |
                bgcolor(Color::Black)
            );
        }

        if (frame < glitchStartFrame) {
            return CenterGameView(
                vbox({
                    filler(),
                    text(""),
                    filler()
                }) |
                bgcolor(Color::Black)
            );
        }

        //Caos cinematografico que termina formando MAZE
        if (frame < titleRevealFrame) {
            const int glitchFrame = frame - glitchStartFrame;

            const std::string backgroundSymbols =
                "01#@%&*+<>[]{}|/\\?X$";

            const std::string logoSymbols =
                "MAZEGRIDEXITROUTEENERGYTRAP";

            const std::array<std::string, 6> logoMask = {
                "XXXX   XXXX XXXXXX XXXXXXXXXXXXXXXX",
                "XXXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                "XXXXXXXXXXXXXXXXXXX  XXXXX XXXXXX  ",
                "XXXXXXXXXXXXXXXXXXX XXXXX  XXXXXX  ",
                "XXX XXX XXXXXX  XXXXXXXXXXXXXXXXXXX",
                "XXX     XXXXXX  XXXXXXXXXXXXXXXXXXX"
            };
            std::vector<std::string> screenMatrix(
                gameViewHeight,
                std::string(gameViewWidth, '0')
            );

            std::vector<std::string> logoActive(
                gameViewHeight,
                std::string(gameViewWidth, '0')
            );

            for (int row = 0; row < gameViewHeight; ++row) {
                for (int col = 0; col < gameViewWidth; ++col) {
                    const std::size_t symbolIndex = (
                        row * 31 +
                        col * 17 +
                        glitchFrame * 13 +
                        row * col
                    ) % backgroundSymbols.size();

                    screenMatrix[row][col] =
                        backgroundSymbols[symbolIndex];
                }
            }

            const int logoHeight =
                static_cast<int>(logoMask.size());

            const int logoWidth =
                static_cast<int>(logoMask[0].size());

            const int logoStartRow =
                (gameViewHeight - logoHeight) / 2;

            const int logoStartCol =
                (gameViewWidth - logoWidth) / 2;

            if (frame >= logoFormationStartFrame) {
                int formationProgress =
                    (
                        (frame - logoFormationStartFrame) *
                        100
                    ) /
                    (
                        titleRevealFrame -
                        logoFormationStartFrame
                    );

                formationProgress =
                    std::clamp(formationProgress, 0, 100);

                for (int row = 0; row < logoHeight; ++row) {
                    for (int col = 0; col < logoWidth; ++col) {
                        if (logoMask[row][col] != 'X') {
                            continue;
                        }

                        const int cellThreshold = (
                            row * 23 +
                            col * 11
                        ) % 100;

                        if (formationProgress < cellThreshold) {
                            continue;
                        }

                        const int screenRow =
                            logoStartRow + row;

                        const int screenCol =
                            logoStartCol + col;

                        const std::size_t logoSymbolIndex = (
                            row * 13 +
                            col * 7 +
                            glitchFrame
                        ) % logoSymbols.size();

                        screenMatrix[screenRow][screenCol] =
                            logoSymbols[logoSymbolIndex];

                        logoActive[screenRow][screenCol] =
                            '1';
                    }
                }
            }

            Elements interferenceLines;

            for (int row = 0; row < gameViewHeight; ++row) {
                Elements rowElements;

                for (int col = 0; col < gameViewWidth; ++col) {
                    const std::string character(
                        1,
                        screenMatrix[row][col]
                    );

                    if (logoActive[row][col] == '1') {
                        rowElements.push_back(
                            text(character) |
                            color(Color::CyanLight) |
                            bold
                        );
                    }
                    else {
                        rowElements.push_back(
                            text(character) |
                            color(Color::GreenLight)
                        );
                    }
                }

                interferenceLines.push_back(
                    hbox(rowElements)
                );
            }

            return CenterGameView(
                vbox(interferenceLines) |
                bgcolor(Color::Black)
            );
        }

        //Titulo MAZE ya completamente formado
        const std::array<std::string, 6> mazeTitle = {
            "███╗   ███╗ █████╗ ███████╗███████╗",
            "████╗ ████║██╔══██╗╚══███╔╝██╔════╝",
            "██╔████╔██║███████║  ███╔╝ █████╗  ",
            "██║╚██╔╝██║██╔══██║ ███╔╝  ██╔══╝  ",
            "██║ ╚═╝ ██║██║  ██║███████╗███████╗",
            "╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝"
        };

        Elements titleLines;

        for (const auto& line : mazeTitle) {
            titleLines.push_back(
                text(line) |
                color(Color::CyanLight) |
                bold |
                center
            );
        }

        //Primero el titulo se queda solo
        if (frame >= subtitleFrame) {
            titleLines.push_back(text(""));

            titleLines.push_back(
                text("ESCAPE PROTOCOL ACTIVE") |
                color(Color::GreenLight) |
                bold |
                center
            );
        }

        //Luego aparece la carga
        if (frame >= loadingStartFrame) {
            int loadingProgress =
                (
                    (frame - loadingStartFrame) *
                    100
                ) /
                (
                    loadingCompleteFrame -
                    loadingStartFrame
                );

            loadingProgress =
                std::clamp(loadingProgress, 0, 100);

            std::string loadingMessage;

            if (loadingProgress < 25) {
                loadingMessage =
                    "LOADING GRID...";
            }
            else if (loadingProgress < 50) {
                loadingMessage =
                    "LOADING NAVIGATION SYSTEM...";
            }
            else if (loadingProgress < 75) {
                loadingMessage =
                    "LOADING ESCAPE PROTOCOL...";
            }
            else if (loadingProgress < 100) {
                loadingMessage =
                    "FINALIZING SYSTEM...";
            }
            else {
                loadingMessage =
                    "SYSTEM READY";
            }

            constexpr int barWidth = 36;

            const int filledWidth =
                (loadingProgress * barWidth) / 100;

            std::string loadingBar = "[";

            for (int i = 0; i < barWidth; ++i) {
                if (i < filledWidth) {
                    loadingBar += "█";
                }
                else {
                    loadingBar += "░";
                }
            }

            loadingBar += "] ";
            loadingBar += std::to_string(loadingProgress);
            loadingBar += "%";

            titleLines.push_back(text(""));

            titleLines.push_back(
                text(loadingMessage) |
                color(Color::CyanLight) |
                center
            );

            titleLines.push_back(
                text(loadingBar) |
                color(
                    loadingProgress == 100
                        ? Color::GreenLight
                        : Color::CyanLight
                ) |
                bold |
                center
            );
        }

        return CenterGameView(
            vbox({
                filler(),
                vbox(titleLines) |
                size(WIDTH, EQUAL, gameViewWidth),
                filler()
            }) |
            bgcolor(Color::Black)
        );
    }
}

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
    ftxui::ScreenInteractive& screen
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
        [&](const ftxui::Event& event){
            std::optional<UICommand> command =
                ui.KeyMapping(event);

            if (!command.has_value()){
                return false;
            }

            if (command == UICommand::up){
                StepResult result =
                    environment.step(Action::up);

                recentEvents = result.events;
            }

            if (command == UICommand::down){
                StepResult result =
                    environment.step(Action::down);

                recentEvents = result.events;
            }

            if (command == UICommand::right){
                StepResult result =
                    environment.step(Action::right);

                recentEvents = result.events;
            }

            if (command == UICommand::left){
                StepResult result =
                    environment.step(Action::left);

                recentEvents = result.events;
            }

            if (command == UICommand::wait){
                StepResult result =
                    environment.step(Action::wait);

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
    using namespace std::chrono_literals;

    auto grid = CreateScenario();
    Agent player = CreatePlayer();
    GameRules rules = rulesFor(Difficulty::standard);

    NavigationEnvironment<20,30> environment(
        grid,
        player,
        rules
    );

    ConsoleUI ui(RenderMode::emoji);

    std::vector<NavigationEvent> recentEvents;

    ftxui::ScreenInteractive screen =
        ftxui::ScreenInteractive::Fullscreen();

    ftxui::Component gameComponent =
        CreateGameComponent(
            environment,
            ui,
            recentEvents,
            screen
        );

    std::atomic<bool> startupActive{true};

    int frame = 0;

    ftxui::Component applicationRenderer =
        ftxui::Renderer(
            [&]{
                if (startupActive) {
                    return RenderStartupFrame(frame);
                }

                return gameComponent->Render();
            }
        );

    ftxui::Component application =
        ftxui::CatchEvent(
            applicationRenderer,
            [&](const ftxui::Event& event){
                if (startupActive) {
                    if (event == ftxui::Event::Return) {
                        startupActive = false;
                        return true;
                    }

                    return true;
                }

                return gameComponent->OnEvent(event);
            }
        );

    std::thread animationThread([&] {
        while (startupActive) {
            std::this_thread::sleep_for(110ms);

            if (!startupActive) {
                break;
            }

            screen.Post([&] {
                ++frame;

                if (frame >= maxFrames) {
                    startupActive = false;
                }
            });

            screen.Post(ftxui::Event::Custom);
        }
    });

    screen.Loop(application);

    startupActive = false;

    if (animationThread.joinable()) {
        animationThread.join();
    }
}