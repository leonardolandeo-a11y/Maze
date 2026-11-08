#include "circuit_escape/menus.h"
#include "circuit_escape/menusArt.h"
#include "circuit_escape/help.h"
#include "circuit_escape/ui_theme.h"

#include <sstream>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <string>

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>


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

    ftxui::Element RenderOption(std::string_view art, bool selected) {
        using namespace ftxui;
        auto option = RenderArt(art) |
            color(selected ? Color::CyanLight : Color::White);
        if (!selected) return hbox({text("  "), option});
        return hbox({
            vbox({text(" "), text(" "), text("▸ "), text(" ")}) |
                color(Color::CyanLight),
            option,
        });
    }

    int OptionCount(MenuScreen screen) {
        switch (screen) {
            case MenuScreen::Main:
                return 6;
            case MenuScreen::MapSize:
                return static_cast<int>(project2_map_sizes.size()) + 2;
            case MenuScreen::CustomSize:
                return 4;
            case MenuScreen::Scenario:
                return 3;
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

Project2MenuLayout project2_menu_layout_for(int width, int height) noexcept {
    switch (MazeUI::metrics(ftxui::Dimensions{width, height}).layout) {
        case MazeUI::Layout::compact: return Project2MenuLayout::compact;
        case MazeUI::Layout::stacked: return Project2MenuLayout::stacked;
        case MazeUI::Layout::split: return Project2MenuLayout::split;
    }
    return Project2MenuLayout::compact;
}

Menus::Menus(bool project2_mode) : project2_mode_(project2_mode) {
    if (project2_mode_) selectedDifficulty_ = Difficulty::easy;
}

/*---------------------------------------------------------------------
OnEvent() -> Procesa la navegación entre pantallas del menú.

W/S y las flechas cambian la opción seleccionada. Enter confirma
la selección y puede cambiar la pantalla actual, configurar una
opción de la partida o producir un MenuCommand para GameApplication.
---------------------------------------------------------------------*/
bool Menus::OnEvent(ftxui::Event event) {
    using namespace ftxui;

    if (currentScreen_ == MenuScreen::CustomSize) {
        if (event == Event::Escape) {
            currentScreen_ = MenuScreen::MapSize;
            selectedOption_ = 3;
        } else if (event == Event::Tab || event == Event::ArrowDown || event == Event::ArrowRight) {
            selectedOption_ = (selectedOption_ + 1) % 4;
            replaceInput_ = true;
        } else if (event == Event::TabReverse || event == Event::ArrowUp || event == Event::ArrowLeft) {
            selectedOption_ = (selectedOption_ + 3) % 4;
            replaceInput_ = true;
        } else if (event == Event::Return) {
            if (selectedOption_ < 2) { ++selectedOption_; replaceInput_ = true; }
            else if (selectedOption_ == 3) { currentScreen_ = MenuScreen::MapSize; selectedOption_ = 3; }
            else {
                try {
                    selectedMapSize_ = parse_project2_map_size(customRows_, customColumns_);
                    mapsReady_ = false;
                    pendingCommand_ = MenuCommand::generateMaps;
                } catch (const std::invalid_argument&) { /* Live status explains the rejection. */ }
            }
        } else if (selectedOption_ < 2) {
            auto& input = selectedOption_ == 0 ? customRows_ : customColumns_;
            if (event == Event::Backspace || event == Event::Delete) {
                if (replaceInput_) input.clear();
                else if (!input.empty()) input.pop_back();
                replaceInput_ = false;
            } else if (event.is_character()) {
                const auto value = event.character();
                if (!value.empty() && std::all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; })) {
                    if (replaceInput_) input.clear();
                    replaceInput_ = false;
                    if (input.size() + value.size() <= 9) input += value;
                }
            }
        }
        return true;
    }

    const int optionCount = project2_mode_ && currentScreen_ == MenuScreen::Main
        ? 5 : OptionCount(currentScreen_);

    if (event == Event::Tab || event == Event::ArrowDown || event == Event::Character('s') ||
        event == Event::Character('S')) {
        selectedOption_ =
            (selectedOption_ + 1) % optionCount;
        return true;
        }

    if (event == Event::TabReverse || event == Event::ArrowUp || event == Event::Character('w') ||
        event == Event::Character('W')) {
        selectedOption_ =
            (selectedOption_ - 1 + optionCount) % optionCount;
        return true;
        }

    if (event == Event::Return || event == Event::Character('\n')) {
        if (currentScreen_ == MenuScreen::Main) {
            if (project2_mode_) {
                switch (selectedOption_) {
                    case 0: currentScreen_ = MenuScreen::MapSize; break;
                    case 1: currentScreen_ = MenuScreen::Difficulty; break;
                    case 2: currentScreen_ = MenuScreen::RenderMode; break;
                    case 3: currentScreen_ = MenuScreen::Help; break;
                    case 4: pendingCommand_ = MenuCommand::quit; return true;
                    default: return false;
                }
                selectedOption_ = 0;
                return true;
            }
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

        if (currentScreen_ == MenuScreen::MapSize) {
            if (selectedOption_ < static_cast<int>(project2_map_sizes.size())) {
                selectedMapSize_ = project2_map_sizes[selectedOption_];
                validate_project2_map_size(selectedMapSize_);
                mapsReady_ = false;
                pendingCommand_ = MenuCommand::generateMaps;
            }
            if (selectedOption_ == 3) {
                currentScreen_ = MenuScreen::CustomSize;
                replaceInput_ = true;
            } else currentScreen_ = MenuScreen::Main;
            selectedOption_ = 0;
            return true;
        }
        if (currentScreen_ == MenuScreen::Difficulty) {
            if (project2_mode_ && mapsReady_ && selectedOption_ < 3)
                pendingCommand_ = MenuCommand::startGame;
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

                case 2: 
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

/*---------------------------------------------------------------------
Entrega a GameApplication el comando pendiente y lo consume,
evitando procesarlo nuevamente en el siguiente evento.
---------------------------------------------------------------------*/
MenuCommand Menus::takeCommand() {
    MenuCommand command = pendingCommand_;
    pendingCommand_ = MenuCommand::none;
    return command;
}

ScenarioSelection Menus::selectedScenario() const noexcept {
    return selectedScenario_;
}

Difficulty Menus::selectedDifficulty() const noexcept {
    return selectedDifficulty_;
}

RenderMode Menus::selectedRenderMode() const noexcept {
    return selectedRenderMode_;
}

ftxui::Element Menus::Render(std::optional<ftxui::Dimensions> terminal) const {
    using namespace ftxui;

    const auto metrics = MazeUI::metrics(terminal);
    const std::string difficulty = selectedDifficulty_ == Difficulty::easy ? "EASY" :
        selectedDifficulty_ == Difficulty::standard ? "MEDIUM" : "HARD";
    const std::string render_mode = selectedRenderMode_ == RenderMode::ascii ? "ASCII" : "EMOJI";
    auto row = [&](std::string label, int index, std::string subtitle = "") {
        return MazeUI::choice(std::move(label), std::move(subtitle),
                              selectedOption_ == index, metrics);
    };
    auto shell = [&](std::string title, Element body, std::string controls, bool brand = false) {
        return MazeUI::shell(std::move(title), std::move(body), std::move(controls), metrics, brand);
    };
    if (currentScreen_ == MenuScreen::MapSize || currentScreen_ == MenuScreen::CustomSize) {
        const bool custom = currentScreen_ == MenuScreen::CustomSize;
        // Size forms need room for validation. Use one-line field rows in every viewport.
        auto field = [&](std::string label, int index) {
            auto dense = metrics;
            dense.dense = true;
            return MazeUI::choice(std::move(label), "", selectedOption_ == index, dense);
        };
        Elements lines;
        if (custom) {
            lines.push_back(field("Rows:     [ " + customRows_ + " ]", 0));
            lines.push_back(field("Columns:  [ " + customColumns_ + " ]", 1));
            std::string status;
            std::string area = "-";
            try {
                const auto size = parse_project2_map_size(customRows_, customColumns_);
                area = std::to_string(size.rows * size.columns);
                status = ConsoleUI::project2_size_error(size, terminal.value_or(Terminal::Size()));
            } catch (const std::invalid_argument& e) { status = e.what(); }
            lines.push_back(separatorEmpty());
            lines.push_back(MazeUI::chip("CELLS", area));
            lines.push_back(paragraph(status.empty() ? "READY TO GENERATE" : status)
                | color(status.empty() ? MazeUI::success : MazeUI::warning));
            lines.push_back(separatorEmpty());
            lines.push_back(field("GENERATE", 2));
            lines.push_back(field("BACK", 3));
        } else {
            constexpr const char* labels[] = {"SMALL", "RECOMMENDED", "LARGE"};
            for (std::size_t i = 0; i < project2_map_sizes.size(); ++i) {
                const auto size = project2_map_sizes[i];
                lines.push_back(row(std::to_string(size.rows) + " x " +
                    std::to_string(size.columns) + "   " + labels[i], static_cast<int>(i)));
            }
            lines.push_back(row("CUSTOM", 3));
            lines.push_back(row("BACK", 4));
        }
        return shell(custom ? "CUSTOM MAP SIZE" : "SELECT MAP SIZE", vbox(std::move(lines)),
            custom ? "Tab/arrows: field | Enter: confirm | Backspace: erase | Esc: back"
                   : "W/S or arrows: select | Enter: confirm");
    }

    if (project2_mode_ && currentScreen_ == MenuScreen::Help) {
        auto mission = MazeUI::panel("THE ESCAPE", vbox({
            paragraph("Activate switches A and B, then reach the exit with energy remaining."),
            paragraph("The guard moves on even turns. Sharing its cell ends the run."),
            paragraph("Terrain costs energy. Waiting or an invalid move costs 1."),
            paragraph("Batteries recharge; traps drain energy. Finish before the turn limit.")
        }));
        auto controls = MazeUI::panel("CONTROLS", vbox({
            text("WASD / arrows   Move"),
            text("E               Wait one turn"),
            text("H               Route and energy help"),
            text("1 / 2 / 3 / 0   Highlight route in help"),
            text("Z               FULL MAP / DETAIL"),
            text("Q               Quit")
        }));
        Element body = metrics.layout == MazeUI::Layout::split
            ? hbox({mission | flex, text("  "), controls | flex})
            : vbox({mission, controls});
        return shell("HELP", body, "Enter: back | Q: quit");
    }

    if (project2_mode_ && (currentScreen_ == MenuScreen::Main ||
        currentScreen_ == MenuScreen::Difficulty || currentScreen_ == MenuScreen::RenderMode)) {
        const bool main = currentScreen_ == MenuScreen::Main;
        const bool level = currentScreen_ == MenuScreen::Difficulty;
        const std::vector<std::string> labels = main
            ? std::vector<std::string>{"START GAME", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT"}
            : level ? std::vector<std::string>{"EASY", "MEDIUM", "HARD", "BACK"}
                    : std::vector<std::string>{"EMOJI", "ASCII", "BACK"};
        const std::vector<std::string> hints = main
            ? std::vector<std::string>{"Prepare your next escape", "Choose your challenge",
                "Choose map symbols", "Learn the circuit", "Leave the circuit"}
            : level ? std::vector<std::string>{"Lowest verified route cost",
                "Balanced verified route cost", "Highest verified route cost", "Return to menu"}
                    : std::vector<std::string>{"Illustrated map symbols",
                "Portable text symbols", "Return to menu"};
        Elements choices;
        for (std::size_t i = 0; i < labels.size(); ++i)
            choices.push_back(row(labels[i], static_cast<int>(i), hints[i]));
        auto navigation = vbox(std::move(choices));
        auto setup = MazeUI::panel("YOUR RUN", vbox({
            MazeUI::chip("DIFFICULTY", difficulty),
            MazeUI::chip("RENDER", render_mode),
            MazeUI::chip("MAP", map_size_label(selectedMapSize_)),
            MazeUI::chip("STATE", mapsReady_ ? "READY" : "NOT PREPARED",
                         mapsReady_ ? MazeUI::success : MazeUI::muted),
            separatorEmpty(),
            paragraph("Activate both switches. Evade the guard. Find your way out.")
        }));
        Element body = metrics.layout == MazeUI::Layout::split
            ? hbox({
                navigation | size(WIDTH, EQUAL, (metrics.width - 5) / 2),
                text("   "),
                vbox({filler(), setup, separatorEmpty(),
                      MazeUI::circuit(0) | hcenter, filler()}) | flex})
            : vbox({navigation, separatorEmpty(),
                MazeUI::chip("CURRENT", difficulty + " / " + render_mode) | hcenter});
        return shell(main ? "MAIN MENU" : level ? "DIFFICULTY" : "RENDER MODE",
            body, "W/S or arrows: select | Enter: confirm | Q: quit", main);
    }

    auto logo = RenderArt(MenuArt::MAZE_LOGO)
              | color(Color::CyanLight);

    ftxui::Elements options;

    switch (currentScreen_) {
        case MenuScreen::Main:
            if (project2_mode_) {
                options = {
                    RenderOption(MenuArt::START_GAME, selectedOption_ == 0),
                    RenderOption(MenuArt::DIFFICULTY, selectedOption_ == 1),
                    RenderOption(MenuArt::RENDER_MODE, selectedOption_ == 2),
                    RenderOption(MenuArt::HELP, selectedOption_ == 3),
                    RenderOption(MenuArt::QUIT, selectedOption_ == 4),
                };
                break;
            }
            options = {
            RenderOption(MenuArt::START_GAME,  selectedOption_ == 0),
            RenderOption(MenuArt::SCENARIO,    selectedOption_ == 1),
            RenderOption(MenuArt::DIFFICULTY,  selectedOption_ == 2),
            RenderOption(MenuArt::RENDER_MODE, selectedOption_ == 3),
            RenderOption(MenuArt::HELP,        selectedOption_ == 4),
            RenderOption(MenuArt::QUIT,        selectedOption_ == 5),
        };
            break;

        case MenuScreen::CustomSize:
        case MenuScreen::MapSize:
            // Size screens render their compact panel above.
            break;
        case MenuScreen::Scenario:
            options = {
            RenderOption(MenuArt::SCENARIO_1, selectedOption_ == 0),
            RenderOption(MenuArt::SCENARIO_2, selectedOption_ == 1),
            RenderOption(MenuArt::BACK,       selectedOption_ == 2),
        };
            break;

        case MenuScreen::Difficulty:
            options = {
            RenderOption(MenuArt::EASY,     selectedOption_ == 0),
            RenderOption(project2_mode_ ? MenuArt::MEDIUM : MenuArt::STANDARD, selectedOption_ == 1),
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
        project2_mode_ ? text("MAP SIZE: " + map_size_label(selectedMapSize_)) : text(""),
        menuOptions,
    });
    ftxui::Element rightPanel = filler();

    if (currentScreen_ == MenuScreen::Help) {
        Help helpView;
        rightPanel = helpView.RenderMenu();
    }

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
void Menus::generationFinished(bool success) {
    mapsReady_ = success;
    currentScreen_ = success ? MenuScreen::Difficulty : MenuScreen::Main;
    selectedOption_ = 0;
    pendingCommand_ = MenuCommand::none;
}
