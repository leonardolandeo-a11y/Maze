#include "circuit_escape/StartupAnimation.h"
#include <algorithm>

ftxui::Element StartupAnimation::RenderStartupFrame(int frame,
    std::optional<ftxui::Dimensions> terminal) const {
    using namespace ftxui;
    const bool ready = frame >= maxFrames - 1;
    auto body = vbox({
        MazeUI::logo() | hcenter,
        separatorEmpty(),
        text("CIRCUITO DE ESCAPE") | bold | color(MazeUI::ink) | hcenter,
        text("Two switches. One way out.") | color(MazeUI::muted) | hcenter,
        MazeUI::circuit(frame) | hcenter,
        text(ready ? "SYSTEM READY" : "ENTER THE CIRCUIT") |
            color(ready ? MazeUI::success : MazeUI::accent) | hcenter,
    });
    return MazeUI::shell("WELCOME", body, "Enter: continue   |   Q: quit",
                         MazeUI::metrics(terminal));
}
