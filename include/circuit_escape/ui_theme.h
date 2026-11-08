#pragma once

#include "circuit_escape/environment.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <optional>
#include <string>

namespace MazeUI {
inline const auto background = ftxui::Color::RGB(7, 11, 15);
inline const auto surface = ftxui::Color::RGB(12, 20, 27);
inline const auto selected_surface = ftxui::Color::RGB(13, 37, 44);
inline const auto accent = ftxui::Color::RGB(77, 226, 232);
inline const auto ink = ftxui::Color::RGB(224, 234, 238);
inline const auto muted = ftxui::Color::RGB(132, 155, 166);
inline const auto edge = ftxui::Color::RGB(48, 78, 90);
inline const auto success = ftxui::Color::RGB(94, 224, 162);
inline const auto warning = ftxui::Color::RGB(239, 193, 92);
inline const auto error = ftxui::Color::RGB(244, 109, 124);

enum class Layout { compact, stacked, split };
struct Metrics {
    int width;
    int height;
    Layout layout;
    bool dense;
};
Metrics metrics(std::optional<ftxui::Dimensions> terminal = std::nullopt);
ftxui::Element logo();
ftxui::Element panel(std::string title, ftxui::Element content);
ftxui::Element chip(std::string label, std::string value,
                    ftxui::Color tone = accent);
ftxui::Element choice(std::string title, std::string subtitle, bool selected,
                      const Metrics& dimensions);
ftxui::Element shell(std::string section, ftxui::Element body,
                     std::string controls, const Metrics& dimensions,
                     bool brand = false);
ftxui::Element circuit(int frame, ftxui::Color tone = accent);
ftxui::Element result(int frame, int frames, std::string title,
                      std::string description, const Observation& observation,
                      ftxui::Color tone, std::optional<ftxui::Dimensions> terminal = std::nullopt);
}
