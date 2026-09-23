#include "circuit_escape/menus.h"
#include "circuit_escape/menusArt.h"

#include <sstream>
#include <string>

#include <ftxui/dom/elements.hpp>

namespace {

    ftxui::Element RenderArt(std::string_view art) {
        using namespace ftxui;

        Elements lines;
        std::istringstream stream{std::string(art)};
        std::string line;

        while (std::getline(stream, line)) {
            lines.push_back(text(line));
        }

        return vbox(std::move(lines));
    }

    ftxui::Element RenderOption(
        std::string_view art,
        bool selected
    ) {
        using namespace ftxui;

        auto option = RenderArt(art)
            | color(selected ? Color::CyanLight : Color::White);

        if (!selected) {
            return hbox({
                text("  "),
                option,
            });
        }

        return hbox({
            vbox({
                text(" "),
                text(" "),
                text("▸ "),
                text(" "),
            }) | color(Color::CyanLight),
            option,
        });
    }
    int OptionCount(MenuScreen screen) {
        switch (screen) {
            case MenuScreen::Main:
                return 6;
            case MenuScreen::Scenario:
                return 4;
            case MenuScreen::Difficulty:
                return 4;
            case MenuScreen::RenderMode:
                return 3;
            case MenuScreen::Help:
                return 1;
        }

        return 1;
    }

}

bool Menus::OnEvent(ftxui::Event event) {
    using namespace ftxui;

    const int optionCount = OptionCount(currentScreen_);

    if (event == Event::ArrowDown || event == Event::Character('s') ||
        event == Event::Character('S')) {
        selectedOption_ =
            (selectedOption_ + 1) % optionCount;
        return true;
        }

    if (event == Event::ArrowUp || event == Event::Character('w') ||
        event == Event::Character('W')) {
        selectedOption_ =
            (selectedOption_ - 1 + optionCount) % optionCount;
        return true;
        }

    if (event == Event::Return || event == Event::Character('\n')) {
        if (currentScreen_ == MenuScreen::Main) {
            switch (selectedOption_) {
                case 0:
                    pendingCommand_ = MenuCommand::startGame;
                    return true;
                case 1:
                    currentScreen_ = MenuScreen::Scenario;
                    break;
                case 2:
                    currentScreen_ = MenuScreen::Difficulty;
                    break;
                case 3:
                    currentScreen_ = MenuScreen::RenderMode;
                    break;
                case 4:
                    currentScreen_ = MenuScreen::Help;
                    break;
                case 5:
                    pendingCommand_ = MenuCommand::quit;
                    return true;
                default:
                    return false;
            }

            selectedOption_ = 0;
            return true;
        }

//logica similar a currentScreen::MenuScreen::Main aplicada a dificultad
        if (currentScreen_ == MenuScreen::Difficulty) {
            switch (selectedOption_) {
                case 0:
                    selectedDifficulty_ = Difficulty::easy;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                case 1:
                    selectedDifficulty_ = Difficulty::standard;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                case 2:
                    selectedDifficulty_ = Difficulty::hard;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                case 3: 
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                default:
                    return false;
            }
        }

        //logica similar a currentScreen::MenuScreen::Main aplicada a Escenario
        if (currentScreen_ == MenuScreen::Scenario) {
            switch(selectedOption_) {
                case 0:
                    selectedScenario_ = ScenarioSelection::scenario1;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;
                case 1:
                    selectedScenario_ = ScenarioSelection::scenario2;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;
                case 2:
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;
                default:
                    return false;
            }
        }
        //lo mismo para renderizacion
        if (currentScreen_ == MenuScreen::RenderMode) {
            switch (selectedOption_) {
                case 0:
                    selectedRenderMode_ = RenderMode::emoji;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                case 1:
                    selectedRenderMode_ = RenderMode::ascii;
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                case 2: // este es BACK
                    currentScreen_ = MenuScreen::Main;
                    selectedOption_ = 0;
                    return true;

                default:
                    return false;
            }
        }
        if (selectedOption_ == OptionCount(currentScreen_) - 1) {
            currentScreen_ = MenuScreen::Main;
            selectedOption_ = 0;
            return true;
        }
    }
    return false;
}

//Metodo agregado para que GameApplication pueda leer el comando recibido desde el menu
MenuCommand Menus::takeCommand() {
    MenuCommand command = pendingCommand_;
    pendingCommand_ = MenuCommand::none;
    return command;
}

//Metodo agregado para que desde el menu se pueda cambiar de escenario
ScenarioSelection Menus::selectedScenario() const noexcept {
    return selectedScenario_;
}

//Metodo agregado para que desde el menu se pueda cambiar de dificultad
Difficulty Menus::selectedDifficulty() const noexcept {
    return selectedDifficulty_;
}
//lo mismo que los otros para renderizar
RenderMode Menus::selectedRenderMode() const noexcept {
    return selectedRenderMode_;
}

ftxui::Element Menus::Render() const {
    using namespace ftxui;

    auto logo = RenderArt(MenuArt::MAZE_LOGO)
              | color(Color::CyanLight);

    ftxui::Elements options;

    switch (currentScreen_) {
        case MenuScreen::Main:
            options = {
            RenderOption(MenuArt::START_GAME,  selectedOption_ == 0),
            RenderOption(MenuArt::SCENARIO,    selectedOption_ == 1),
            RenderOption(MenuArt::DIFFICULTY,  selectedOption_ == 2),
            RenderOption(MenuArt::RENDER_MODE, selectedOption_ == 3),
            RenderOption(MenuArt::HELP,        selectedOption_ == 4),
            RenderOption(MenuArt::QUIT,        selectedOption_ == 5),
        };
            break;

        case MenuScreen::Scenario:
            options = {
            RenderOption(MenuArt::SCENARIO_1, selectedOption_ == 0),
            RenderOption(MenuArt::SCENARIO_2, selectedOption_ == 1),
            RenderOption(MenuArt::RANDOM,     selectedOption_ == 2),
            RenderOption(MenuArt::BACK,       selectedOption_ == 3),
        };
            break;

        case MenuScreen::Difficulty:
            options = {
            RenderOption(MenuArt::EASY,     selectedOption_ == 0),
            RenderOption(MenuArt::STANDARD, selectedOption_ == 1),
            RenderOption(MenuArt::HARD,     selectedOption_ == 2),
            RenderOption(MenuArt::BACK,     selectedOption_ == 3),
        };
            break;

        case MenuScreen::RenderMode:
            options = {
            RenderOption(MenuArt::EMOJI, selectedOption_ == 0),
            RenderOption(MenuArt::ASCII, selectedOption_ == 1),
            RenderOption(MenuArt::BACK,  selectedOption_ == 2),
        };
            break;

        case MenuScreen::Help:
            options = {
            RenderOption(MenuArt::BACK, selectedOption_ == 0),
        };
            break;
    }

    auto menuOptions = vbox(std::move(options));

    auto leftPanel = vbox({
        text(""),
        text(""),
        text(""),
        text(""),
        logo,
        separatorEmpty(),
        separatorEmpty(),
        separatorEmpty(),
        separatorEmpty(),
        separatorEmpty(),
        separatorEmpty(),
        menuOptions,
    });
    auto rightPanel = filler();

    return hbox({
        text("                        "), // 24 columnas
        leftPanel,
        filler(),
        rightPanel | flex,
    })
    | flex
    | size(WIDTH, GREATER_THAN, 1)
    | size(HEIGHT, GREATER_THAN, 1)
    | bgcolor(Color::Black);
}