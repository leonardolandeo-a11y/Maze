#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/UI_Layout.h"

#include <array>
#include <string>

#include <ftxui/screen/color.hpp>

ftxui::Element VictoryAnimation::RenderIntroSequence(int frame) const{
    const std::string message =
        frame < 10
            ? "ESCAPE ROUTE CONFIRMED"
            : "EXIT REACHED";

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::text(message) |
            ftxui::color(ftxui::Color::GreenLight) |
            ftxui::bold |
            ftxui::center,
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element VictoryAnimation::RenderCelebrationSequence(int frame) const{
    const std::string symbol =
        frame % 2 == 0
            ? "*"
            : "+";

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::text(symbol + "  AGENT ESCAPED  " + symbol) |
            ftxui::color(ftxui::Color::GreenLight) |
            ftxui::bold |
            ftxui::center,
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element VictoryAnimation::RenderVictoryScreen() const{
    const std::array<std::string, 6> victoryTitle = {
        "██╗   ██╗██╗ ██████╗████████╗ ██████╗ ██████╗ ██╗   ██╗",
        "██║   ██║██║██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗╚██╗ ██╔╝",
        "██║   ██║██║██║        ██║   ██║   ██║██████╔╝ ╚████╔╝ ",
        "╚██╗ ██╔╝██║██║        ██║   ██║   ██║██╔══██╗  ╚██╔╝  ",
        " ╚████╔╝ ██║╚██████╗   ██║   ╚██████╔╝██║  ██║   ██║   ",
        "  ╚═══╝  ╚═╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   "
    };

    ftxui::Elements titleLines;

    for (const auto& line : victoryTitle){
        titleLines.push_back(
            ftxui::text(line) |
            ftxui::color(ftxui::Color::GreenLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text("ESCAPE PROTOCOL COMPLETE") |
        ftxui::color(ftxui::Color::White) |
        ftxui::bold |
        ftxui::center
    );

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(titleLines),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element VictoryAnimation::RenderVictoryFrame(int frame) const{
    if (frame < introEndFrame){
        return RenderIntroSequence(frame);
    }

    if (frame < celebrationEndFrame){
        return RenderCelebrationSequence(frame);
    }

    return RenderVictoryScreen();
}