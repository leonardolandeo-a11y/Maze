#pragma once
#include "circuit_escape/ui_theme.h"

// Presentation only; the application owns the existing frame/refresh loop.
class VictoryAnimation {
public:
    static constexpr int maxFrames = 36;
    ftxui::Element RenderVictoryFrame(int frame, const Observation& observation,
        std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
};
