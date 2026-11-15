#pragma once

#include "circuit_escape/ui_theme.h"
#include "circuit_escape/help_controls.h"
#include <ftxui/screen/string.hpp>
#include <algorithm>
#include <sstream>

namespace MazeUI {
// Wrap by terminal columns (rather than UTF-8 bytes), with a predictable
// height so a help panel can cover the board without resizing its camera.
inline ftxui::Element help_text(const std::string& value, int width,
                                ftxui::Color tone = ink) {
    ftxui::Elements lines;
    std::istringstream words(value);
    std::string word, line;
    while (words >> word) {
        if (!line.empty() && ftxui::string_width(line + " " + word) > std::max(1, width)) {
            lines.push_back(ftxui::text(line) | ftxui::color(tone));
            line.clear();
        }
        if (!line.empty()) line += " ";
        line += word;
    }
    lines.push_back(ftxui::text(line) | ftxui::color(tone));
    return ftxui::vbox(std::move(lines));
}
}
