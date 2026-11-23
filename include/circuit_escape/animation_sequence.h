#pragma once

#include "circuit_escape/CinematicStartupAnimation.h"
#include "circuit_escape/CinematicVictoryAnimation.h"
#include "circuit_escape/CinematicDeathAnimation.h"
#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/DeathAnimation.h"

// Shared by both application controllers. Counters are advanced only by their
// existing event-loop callbacks; these helpers neither wait nor mutate gameplay.
namespace AnimationSequence {
inline constexpr int startupFrames = CinematicStartupAnimation::maxFrames;
inline constexpr int victoryFrames = CinematicVictoryAnimation::maxFrames + VictoryAnimation::maxFrames;
inline constexpr int deathFrames = CinematicDeathAnimation::maxFrames + DeathAnimation::maxFrames;

constexpr bool startupComplete(int frame) { return frame >= startupFrames; }
constexpr bool victoryComplete(int frame) { return frame >= victoryFrames - 1; }
constexpr bool deathComplete(int frame) { return frame >= deathFrames - 1; }

inline ftxui::Element startup(int frame, std::optional<ftxui::Dimensions> terminal = std::nullopt) {
    return CinematicStartupAnimation{}.RenderFrame(frame, terminal);
}
inline ftxui::Element victory(int frame, const Observation& observation,
                              std::optional<ftxui::Dimensions> terminal = std::nullopt) {
    if (frame < CinematicVictoryAnimation::maxFrames)
        return CinematicVictoryAnimation{}.RenderFrame(frame, terminal);
    return VictoryAnimation{}.RenderVictoryFrame(frame - CinematicVictoryAnimation::maxFrames,
                                                 observation, terminal);
}
inline ftxui::Element death(int frame, EndReason reason, const Observation& observation,
                            std::optional<ftxui::Dimensions> terminal = std::nullopt) {
    if (frame < CinematicDeathAnimation::maxFrames)
        return CinematicDeathAnimation{}.RenderFrame(frame, terminal);
    return DeathAnimation{}.RenderDeathFrame(frame - CinematicDeathAnimation::maxFrames,
                                             reason, observation, terminal);
}
}
