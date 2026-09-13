#pragma once

#include "circuit_escape/environment.h"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <optional>
#include <span>

enum class RenderMode {
    emoji,
    ascii
};

enum class UICommand{
    up,
    down,
    left,
    right,
    wait,
    quit,
};
class ConsoleUI {
private:
    RenderMode mode_;

    ftxui::Element RenderCell(const Cell& cell, bool IsAgent) const;
public:
    explicit ConsoleUI(RenderMode mode = RenderMode::emoji);

    ftxui::Element render(
        const NavigationEnvironment<20, 30>& environment,
        std::span<const NavigationEvent> recentEvents) const;
    
    std::optional<UICommand> KeyMapping(const ftxui::Event& event) const;



};