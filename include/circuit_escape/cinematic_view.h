#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <optional>

namespace CinematicView {
// The original artists' logical stage; never changes with terminal resize.
ftxui::Element Stage(ftxui::Element content);
// Fits the whole stage proportionally; does not trim the timeline or crop actors.
ftxui::Element Fit(ftxui::Element scene,
                   std::optional<ftxui::Dimensions> terminal = std::nullopt);
}
