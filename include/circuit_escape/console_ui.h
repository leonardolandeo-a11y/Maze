#pragma once

#include "circuit_escape/environment.h"

#include <ftxui/dom/elements.hpp>

#include <span>

enum class RenderMode {
    emoji,
    ascii
};

class ConsoleUI {
public:
    explicit ConsoleUI(RenderMode mode = RenderMode::emoji);

    ftxui::Element render(
        const NavigationEnvironment<20, 30>& environment,
        std::span<const NavigationEvent> recentEvents) const;

private:
    RenderMode mode_;
};