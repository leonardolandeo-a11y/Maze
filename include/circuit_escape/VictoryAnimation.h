#pragma once

#include <ftxui/dom/elements.hpp>

class VictoryAnimation{
    static constexpr int walkEndFrame = 24;
    static constexpr int turnEndFrame = 34;
    static constexpr int raiseArmEndFrame = 46;
    static constexpr int celebrationEndFrame = 78;
    static constexpr int victoryTitleFrame = 90;

    ftxui::Element RenderWalkingSequence(int frame) const;
    ftxui::Element RenderTurnSequence(int frame) const;
    ftxui::Element RenderRaiseArmSequence(int frame) const;
    ftxui::Element RenderCelebrationSequence(int frame) const;
    ftxui::Element RenderVictoryScreen(int frame) const;

public:
    static constexpr int maxFrames = 110;

    ftxui::Element RenderVictoryFrame(int frame) const;
};