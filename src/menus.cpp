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

    return false;
}

ftxui::Element Menus::Render() const {
    using namespace ftxui;

    auto logo = RenderArt(MenuArt::MAZE_LOGO)
              | color(Color::CyanLight);

    auto options = vbox({
        RenderOption(MenuArt::START_GAME,  selectedOption_ == 0),
        RenderOption(MenuArt::SCENARIO,    selectedOption_ == 1),
        RenderOption(MenuArt::DIFFICULTY,  selectedOption_ == 2),
        RenderOption(MenuArt::RENDER_MODE, selectedOption_ == 3),
        RenderOption(MenuArt::HELP,        selectedOption_ == 4),
        RenderOption(MenuArt::QUIT,        selectedOption_ == 5),
    });

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
        options,
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