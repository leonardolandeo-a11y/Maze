#include "circuit_escape/MapGenerationAnimation.h"
#include <algorithm>

ftxui::Element MapGenerationAnimation::RenderFrame(int frame, Project2MapSize map_size,
    std::optional<ftxui::Dimensions> terminal) const {
    using namespace ftxui;
    auto body = vbox({
        MazeUI::logo() | hcenter,
        separatorEmpty(),
        text("PREPARING THE ESCAPE") | bold | color(MazeUI::ink) | hcenter,
        text("GENERATING " + std::to_string(map_size.rows) + " x " +
             std::to_string(map_size.columns)) | color(MazeUI::accent) | hcenter,
        MazeUI::circuit(frame) | hcenter,
        hbox({spinner(0, std::max(0, frame) / 2), text(" PLEASE WAIT...")}) |
            color(MazeUI::muted) | hcenter,
    });
    return MazeUI::shell("STANDBY", body, "Q: quit (waits for current batch)",
                         MazeUI::metrics(terminal));
}
