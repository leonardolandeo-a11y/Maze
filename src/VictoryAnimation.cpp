#include "circuit_escape/VictoryAnimation.h"

ftxui::Element VictoryAnimation::RenderVictoryFrame(int frame,
    const Observation& observation, std::optional<ftxui::Dimensions> terminal) const {
    return MazeUI::result(frame, maxFrames, "VICTORY", "Circuit complete. You escaped.",
                          observation, MazeUI::success, terminal);
}
