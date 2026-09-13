#pragma once

#include "circuit_escape/environment.h"

#include <ftxui/dom/elements.hpp>

class DeathAnimation{
    static constexpr int gameViewWidth = 90;
    static constexpr int gameViewHeight = 20;

    static constexpr int glitchEndFrame = 20;
    static constexpr int blankEndFrame = 24;
    static constexpr int failureEndFrame = 40;

    ftxui::Element RenderGlitchSequence(int frame) const;
    ftxui::Element RenderBlankTransition() const;
    ftxui::Element RenderSystemFailure(int frame, EndReason reason) const;
    ftxui::Element RenderGameOver(EndReason reason) const;

public:
    static constexpr int maxFrames = 70;

    ftxui::Element RenderDeathFrame(int frame, EndReason reason) const;
};