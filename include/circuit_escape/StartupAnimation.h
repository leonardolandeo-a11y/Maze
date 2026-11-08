#pragma once
#include "circuit_escape/ui_theme.h"

// Presentation only; the application owns the existing frame/refresh loop.
class StartupAnimation {
public:
    static constexpr int maxFrames = 36;
    ftxui::Element RenderStartupFrame(int frame,
        std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
};
