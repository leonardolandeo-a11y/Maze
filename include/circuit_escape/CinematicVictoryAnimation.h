#pragma once
#include <optional>
#include <ftxui/screen/terminal.hpp>

#include <ftxui/dom/elements.hpp>
#include "circuit_escape/environment.h"
class CinematicVictoryAnimation{
    static constexpr int walkEndFrame = 24;
    static constexpr int turnEndFrame = 34;
    static constexpr int raiseArmEndFrame = 46;
    static constexpr int celebrationEndFrame = 78;
    ftxui::Element RenderWalkingSequence(int frame) const;
    ftxui::Element RenderTurnSequence(int frame) const;
    ftxui::Element RenderRaiseArmSequence(int frame) const;
    ftxui::Element RenderCelebrationSequence(int frame) const;

public:
    static constexpr int maxFrames = celebrationEndFrame;

    ftxui::Element RenderFrame(int frame,
        std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
};
