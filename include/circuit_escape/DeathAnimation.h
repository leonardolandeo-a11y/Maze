#pragma once
#include "circuit_escape/ui_theme.h"

// Presentation only; the application owns the existing frame/refresh loop.
class DeathAnimation {
public:
    static constexpr int maxFrames = 36;
    ftxui::Element RenderDeathFrame(int frame, EndReason reason, const Observation& observation,
        std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
};
