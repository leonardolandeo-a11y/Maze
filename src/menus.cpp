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

}

ftxui::Element Menus::Render() const {
    using namespace ftxui;

    auto logo = RenderArt(MenuArt::MAZE_LOGO)
              | color(Color:: CyanLight);

    auto options = vbox({
        RenderArt(MenuArt::START_GAME),
        RenderArt(MenuArt::SCENARIO),
        RenderArt(MenuArt::DIFFICULTY),
        RenderArt(MenuArt::RENDER_MODE),
        RenderArt(MenuArt::HELP),
        RenderArt(MenuArt::QUIT),
    }) | color(Color::White);

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