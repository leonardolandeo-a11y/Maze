#pragma once

#include <ftxui/dom/elements.hpp>

class VictoryAnimation{
    static constexpr int introEndFrame = 20;
    static constexpr int celebrationEndFrame = 50;
    static constexpr int victoryTitleFrame = 64;

    ftxui::Element RenderIntroSequence(int frame) const;
    ftxui::Element RenderCelebrationSequence(int frame) const;
    ftxui::Element RenderVictoryScreen() const;

public:
    static constexpr int maxFrames = 90;

    ftxui::Element RenderVictoryFrame(int frame) const;
};