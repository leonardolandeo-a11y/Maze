#include "circuit_escape/DeathAnimation.h"

ftxui::Element DeathAnimation::RenderDeathFrame(int frame, EndReason reason,
    const Observation& observation, std::optional<ftxui::Dimensions> terminal) const {
    const char* description = "RUN ENDED";
    switch (reason) {
        case EndReason::captured: description = "CAPTURED BY THE GUARD"; break;
        case EndReason::noEnergy: description = "ENERGY DEPLETED"; break;
        case EndReason::turnLimit: description = "TURN LIMIT EXCEEDED"; break;
        default: break;
    }
    return MazeUI::result(frame, maxFrames, "GAME OVER", description,
                          observation, MazeUI::error, terminal);
}
