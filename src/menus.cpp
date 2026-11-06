#include "circuit_escape/menus.h"
#include "circuit_escape/menusArt.h"
#include "circuit_escape/help.h"

#include <sstream>
#include <array>
#include <algorithm>
#include <stdexcept>
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
    // Project 2 uses the same phosphor palette and block logo as the intro.
    ftxui::Element ArcadeRow(std::string label, bool selected) {
        using namespace ftxui;
        auto row = hbox({text(selected ? " ▸ " : "   "), text(label), filler(),
                         text(selected ? " ◂ " : "   ")});
        return selected ? row | bold | color(Color::Black) | bgcolor(Color::CyanLight)
                        : row | color(Color::White);
    }

    ftxui::Element ArcadePanel(std::string title, ftxui::Elements content,
                              std::string controls) {
        using namespace ftxui;
        Elements lines{
            text(" ESCAPE PROTOCOL / " + title) | bold | color(Color::CyanLight),
            separator() | color(Color::Cyan),
        };
        for (auto& line : content) lines.push_back(std::move(line));
        lines.push_back(separator() | color(Color::Cyan));
        lines.push_back(paragraph(std::move(controls)) | color(Color::GrayLight));
        return vbox(std::move(lines)) | size(WIDTH, EQUAL, 52) | borderRounded |
            color(Color::CyanLight) | center | bgcolor(Color::Black);
    }

    ftxui::Element ArcadeLogo() {
        using namespace ftxui;
        Elements lines;
        std::istringstream stream{std::string(MenuArt::MAZE_LOGO)};
        std::string line;
        while (std::getline(stream, line)) {
            // The legacy art includes blank braille padding on its last line.
            const auto padding = line.find("   ", line.find("╚═╝"));
            if (padding != std::string::npos) line.erase(padding);
            if (!line.empty()) lines.push_back(text(line) | hcenter);
        }
        return vbox(std::move(lines)) | bold | color(Color::CyanLight);
    }

    std::string DifficultyLabel(Difficulty difficulty) {
        switch (difficulty) {
            case Difficulty::easy: return "EASY";
            case Difficulty::standard: return "MEDIUM";
            case Difficulty::hard: return "HARD";
        }
        return "EASY";
    }

    std::string RenderModeLabel(RenderMode mode) {
        return mode == RenderMode::emoji ? "EMOJI" : "ASCII";
    }

    ftxui::Element Project2Row(std::string label, bool selected,
                               std::string hint = {}) {
        using namespace ftxui;
        auto marker = text(selected ? "> " : "  ") |
            color(selected ? Color::CyanLight : Color::GrayLight);
        auto name = text(std::move(label)) |
            color(selected ? Color::CyanLight : Color::White);
        if (selected) name = name | bold;
        auto tail = hint.empty()
            ? (selected ? text("SELECT") | color(Color::GreenLight) : text(""))
            : text(std::move(hint)) | color(selected ? Color::GreenLight : Color::GrayLight);
        return hbox({
            text("  "),
            std::move(marker),
            std::move(name),
            filler(),
            std::move(tail),
            text("  "),
        });
    }

    ftxui::Element StatusLine(std::string key, std::string value, ftxui::Color value_color) {
        using namespace ftxui;
        return hbox({
            text("  "),
            text(std::move(key)) | color(Color::GrayLight),
            filler(),
            text(std::move(value)) | bold | color(value_color),
            text("  "),
        });
    }

    struct Project2FrameMetrics {
        int width;
        int height;
        Project2MenuLayout mode;
        bool show_logo;
    };

    Project2FrameMetrics Project2Metrics() {
        using namespace ftxui;
        const auto terminal = Terminal::Size();
        const int terminal_width = terminal.dimx > 0 ? terminal.dimx : 120;
        const int terminal_height = terminal.dimy > 0 ? terminal.dimy : 32;
        const auto mode = project2_menu_layout_for(terminal_width, terminal_height);

        const int desired_width = std::max(40, terminal_width * 9 / 10);
        const int available_width = terminal_width > 4 ? terminal_width - 2 : terminal_width;
        const int frame_width = std::max(1,
            std::min({142, desired_width, std::max(1, available_width)}));

        const int desired_height = std::max(16, terminal_height * 4 / 5);
        const int available_height = terminal_height > 4 ? terminal_height - 2 : terminal_height;
        const int frame_height = std::max(1,
            std::min({38, desired_height, std::max(1, available_height)}));

        return {
            frame_width,
            frame_height,
            mode,
            mode != Project2MenuLayout::compact && frame_width >= 56 && frame_height >= 23
        };
    }

    ftxui::Element Project2Shell(std::string section,
                                 ftxui::Element body,
                                 std::string controls,
                                 bool show_brand = true) {
        using namespace ftxui;
        const auto metrics = Project2Metrics();
        const bool compact = metrics.mode == Project2MenuLayout::compact;

        Elements content;
        if (!compact) {
            content.push_back(hbox({
                text("ESCAPE PROTOCOL") | bold | color(Color::CyanLight),
                filler(),
                text(std::move(section)) | color(Color::GrayLight),
            }));
            content.push_back(separator() | color(Color::Cyan));
        }

        if (show_brand) {
            if (metrics.show_logo) {
                content.push_back(separatorEmpty());
                content.push_back(ArcadeLogo());
                content.push_back(text("CIRCUITO DE ESCAPE") |
                    hcenter | color(Color::GrayLight));
            } else {
                content.push_back(text("MAZE // CIRCUITO DE ESCAPE") |
                    bold | hcenter | color(Color::CyanLight));
            }
        }

        content.push_back(vbox({
            filler(),
            std::move(body),
            filler(),
        }) | flex);

        content.push_back(separator() | color(Color::Cyan));
        content.push_back(text(std::move(controls)) |
            hcenter | color(Color::GrayLight));

        return vbox(std::move(content)) |
            size(WIDTH, EQUAL, metrics.width) |
            size(HEIGHT, EQUAL, metrics.height) |
            borderRounded |
            color(Color::CyanLight) |
            center |
            bgcolor(Color::Black);
    }

    ftxui::Element Project2MainMenu(Project2MapSize map_size,
                                    Difficulty difficulty,
                                    RenderMode render_mode,
                                    bool maps_ready,
                                    int selected_option) {
        using namespace ftxui;
        const auto metrics = Project2Metrics();
        const bool split = metrics.mode == Project2MenuLayout::split;
        const bool compact = metrics.mode == Project2MenuLayout::compact;

        Elements menu_rows;
        const std::array<std::string, 5> labels{
            "START GAME", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT"
        };
        const bool spacious = metrics.height >= 30;
        for (std::size_t index = 0; index < labels.size(); ++index) {
            menu_rows.push_back(Project2Row(labels[index],
                selected_option == static_cast<int>(index)));
            if (spacious && index + 1 < labels.size()) {
                menu_rows.push_back(separatorEmpty());
            }
        }

        auto menu_panel = vbox({
            text("COMMANDS") | bold | color(Color::White),
            separator() | color(Color::GrayDark),
            vbox(std::move(menu_rows)),
        }) | flex;

        const std::string map_text = std::to_string(map_size.rows) + " x " +
            std::to_string(map_size.columns);
        Elements status_rows{
            text("CURRENT SETUP") | bold | color(Color::White),
            separator() | color(Color::GrayDark),
            StatusLine("MAP", map_text, Color::CyanLight),
        };
        if (spacious) status_rows.push_back(separatorEmpty());
        status_rows.push_back(StatusLine("DIFFICULTY", DifficultyLabel(difficulty),
            Color::GreenLight));
        if (spacious) status_rows.push_back(separatorEmpty());
        status_rows.push_back(StatusLine("RENDER", RenderModeLabel(render_mode), Color::White));
        if (spacious) status_rows.push_back(separatorEmpty());
        status_rows.push_back(StatusLine("MAP SET", maps_ready ? "READY" : "NOT GENERATED",
            maps_ready ? Color::GreenLight : Color::YellowLight));
        auto status_panel = vbox(std::move(status_rows)) | flex;

        Element body;
        if (split) {
            body = hbox({
                menu_panel | flex,
                separatorEmpty(),
                separator() | color(Color::GrayDark),
                separatorEmpty(),
                status_panel | flex,
            });
        } else if (compact) {
            body = vbox({
                std::move(menu_panel),
                separator() | color(Color::GrayDark),
                text(map_text + "  |  " + DifficultyLabel(difficulty) + "  |  " +
                    RenderModeLabel(render_mode)) | hcenter | color(Color::GrayLight),
                text(maps_ready ? "MAP SET READY" : "MAP SET NOT GENERATED") |
                    hcenter | color(maps_ready ? Color::GreenLight : Color::YellowLight),
            });
        } else {
            body = vbox({
                std::move(menu_panel),
                separatorEmpty(),
                separator() | color(Color::GrayDark),
                separatorEmpty(),
                std::move(status_panel),
            });
        }

        return Project2Shell("MAIN MENU", std::move(body),
            compact ? "Arrows: move   Enter: select   Q: quit"
                    : "W/S or arrows: navigate   Enter: select   Q: quit");
    }

    ftxui::Element Project2ChoiceMenu(std::string section,
                                      const std::vector<std::string>& labels,
                                      const std::vector<std::string>& descriptions,
                                      int selected_option,
                                      std::string current_value) {
        using namespace ftxui;
        const auto metrics = Project2Metrics();
        Elements rows;
        const bool spacious = metrics.height >= 30;
        for (std::size_t index = 0; index < labels.size(); ++index) {
            rows.push_back(Project2Row(labels[index],
                selected_option == static_cast<int>(index),
                index < descriptions.size() ? descriptions[index] : ""));
            if (spacious && index + 1 < labels.size()) {
                rows.push_back(separatorEmpty());
            }
        }

        auto choices = vbox({
            text(section) | bold | color(Color::White),
            separator() | color(Color::GrayDark),
            vbox(std::move(rows)),
        });

        Element body;
        if (metrics.mode == Project2MenuLayout::split) {
            body = hbox({
                choices | flex,
                separatorEmpty(),
                separator() | color(Color::GrayDark),
                separatorEmpty(),
                vbox({
                    text("CURRENT") | bold | color(Color::White),
                    separator() | color(Color::GrayDark),
                    text(std::move(current_value)) | bold | color(Color::GreenLight),
                    separatorEmpty(),
                    text("Changes apply without altering map generation or gameplay.") |
                        color(Color::GrayLight),
                }) | flex,
            });
        } else {
            body = vbox({
                std::move(choices),
                separatorEmpty(),
                text("CURRENT  " + current_value) | hcenter | color(Color::GreenLight),
            });
        }

        return Project2Shell(section, std::move(body),
            "W/S or arrows: navigate   Enter: confirm   Back: return");
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
    if (width >= 92 && height >= 24) return Project2MenuLayout::split;
    if (width >= 48 && height >= 18) return Project2MenuLayout::stacked;
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

ftxui::Element Menus::Render() const {
    using namespace ftxui;

    if (currentScreen_ == MenuScreen::MapSize || currentScreen_ == MenuScreen::CustomSize) {
        const bool custom = currentScreen_ == MenuScreen::CustomSize;
        Elements lines;
        auto row = [&](std::string label, int index) {
            return Project2Row(std::move(label), selectedOption_ == index);
        };
        lines.push_back(text(custom ? "CUSTOM MAP SIZE" : "SELECT MAP SIZE") |
            bold | hcenter | color(Color::White));
        lines.push_back(text("ROWS × COLUMNS") | hcenter | color(Color::GreenLight));
        lines.push_back(separatorEmpty());
        if (custom) {
            lines.push_back(row("Rows:     [ " + customRows_ + " ]", 0));
            lines.push_back(row("Columns:  [ " + customColumns_ + " ]", 1));
            std::string status;
            std::string area = "—";
            try {
                const auto size = parse_project2_map_size(customRows_, customColumns_);
                area = std::to_string(size.rows * size.columns);
                status = ConsoleUI::project2_size_error(size, Terminal::Size());
            } catch (const std::invalid_argument& e) { status = e.what(); }
            lines.push_back(separatorEmpty());
            lines.push_back(text("CELLS  " + area) | color(Color::White));
            lines.push_back(paragraph(status.empty() ? "✓ READY TO GENERATE" : "! " + status)
                | color(status.empty() ? Color::GreenLight : Color::RedLight));
            lines.push_back(separatorEmpty());
            lines.push_back(row("[ GENERATE ]", 2));
            lines.push_back(row("[ BACK ]", 3));
        } else {
            constexpr const char* labels[] = {"SMALL", "RECOMMENDED", "LARGE"};
            for (std::size_t index = 0; index < project2_map_sizes.size(); ++index) {
                const auto size = project2_map_sizes[index];
                lines.push_back(row(std::to_string(size.rows) + "×" + std::to_string(size.columns) +
                    "      " + labels[index], static_cast<int>(index)));
            }
            lines.push_back(row("CUSTOM      CREATE YOUR SIZE", 3));
            lines.push_back(row("BACK", 4));
        }
        lines.push_back(separatorEmpty());
        return Project2Shell(custom ? "CUSTOM MAP SIZE" : "MAP SIZE",
            vbox(std::move(lines)),
            custom
                ? "Tab/arrows: select   Enter: confirm   Backspace: erase   Esc: back"
                : "W/S or arrows: navigate   Enter: confirm   Back: return");
    }

    if (project2_mode_ && currentScreen_ == MenuScreen::Main) {
        return Project2MainMenu(selectedMapSize_, selectedDifficulty_, selectedRenderMode_,
            mapsReady_, selectedOption_);
    }

    if (project2_mode_ && currentScreen_ == MenuScreen::Difficulty) {
        return Project2ChoiceMenu("DIFFICULTY",
            {"EASY", "MEDIUM", "HARD", "BACK"},
            {"LOWER COST", "BALANCED", "HIGHER COST", ""},
            selectedOption_, DifficultyLabel(selectedDifficulty_));
    }

    if (project2_mode_ && currentScreen_ == MenuScreen::RenderMode) {
        return Project2ChoiceMenu("RENDER MODE",
            {"EMOJI", "ASCII", "BACK"},
            {"RICH SYMBOLS", "PORTABLE", ""},
            selectedOption_, RenderModeLabel(selectedRenderMode_));
    }

    if (project2_mode_ && currentScreen_ == MenuScreen::Help) {
        Help helpView;
        return Project2Shell("HELP", helpView.RenderMenu(),
            "Enter: back   Q: quit", false);
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
