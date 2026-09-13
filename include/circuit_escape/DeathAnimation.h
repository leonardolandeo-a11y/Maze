#pragma once

#include "circuit_escape/environment.h"

#include <ftxui/dom/elements.hpp>

class DeathAnimation{
    static constexpr int gameViewWidth = 90;
    static constexpr int gameViewHeight = 20;

    static constexpr int fallEndFrame = 16;
    static constexpr int lyingEndFrame = 28;
    static constexpr int riseEndFrame = 42;
    static constexpr int turnEndFrame = 50;

    static constexpr int bombDrawEndFrame = 62;
    static constexpr int bombRaiseEndFrame = 72;
    static constexpr int bombInspectEndFrame = 84;

    static constexpr int explosionEndFrame = 94;
    static constexpr int debrisFallEndFrame = 126;
    static constexpr int debrisRestEndFrame = 142;

    ftxui::Element RenderFallingSequence(int frame) const;
    ftxui::Element RenderLyingSequence() const;
    ftxui::Element RenderRiseSequence(int frame) const;
    ftxui::Element RenderTurnSequence(int frame) const;
    ftxui::Element RenderBombSequence(int frame) const;
    ftxui::Element RenderExplosionSequence(int frame) const;
    ftxui::Element RenderDebrisSequence(int frame) const;
    ftxui::Element RenderGameOver(int frame, EndReason reason) const;

public:
    static constexpr int maxFrames = 160;

    ftxui::Element RenderDeathFrame(int frame, EndReason reason) const;
};