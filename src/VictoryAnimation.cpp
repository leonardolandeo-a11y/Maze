#include "circuit_escape/VictoryAnimation.h"

ftxui::Element VictoryAnimation::RenderVictoryFrame(int frame,
    const Observation& observation, std::optional<ftxui::Dimensions> terminal) const {
    return MazeUI::result(frame, maxFrames, "ESCAPE COMPLETED", "¡Has escapado del circuito!",
                          observation, MazeUI::success, terminal);
}
