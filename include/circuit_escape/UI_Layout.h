#pragma once
#include "circuit_escape/ui_theme.h"

namespace UILayout {
inline ftxui::Element CenterGameView(ftxui::Element content) {
    const auto dimensions = MazeUI::metrics();
    return std::move(content) |
        ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, dimensions.width) |
        ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, dimensions.height) |
        ftxui::center | ftxui::bgcolor(MazeUI::background);
}
}
