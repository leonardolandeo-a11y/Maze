#include "circuit_escape/MapGenerationAnimation.h"

#include <algorithm>
#include <array>
#include <string>

ftxui::Element MapGenerationAnimation::RenderFrame(int frame, Project2MapSize map_size) const {
    using namespace ftxui;
    const auto tick = static_cast<unsigned>(std::max(0, frame));
    static constexpr std::array<const char*, 8> spinner{"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧"};
    // A looping activity signal, never a completion bar or a claimed worker phase.
    Elements pulse;
    for (unsigned i = 0; i < 9; ++i) {
        pulse.push_back(text(i == (tick / 2) % 9 ? "◆ " : "· ") |
            color(i == (tick / 2) % 9 ? Color::CyanLight : Color::Cyan));
    }
    return vbox({
        text(" ESCAPE PROTOCOL / STANDBY ") | bold | color(Color::CyanLight) | hcenter,
        separator() | color(Color::Cyan),
        separatorEmpty(),
        text("PREPARING THE ESCAPE") | bold | color(Color::White) | hcenter,
        separatorEmpty(),
        text("GENERATING " + std::to_string(map_size.rows) + " × " +
            std::to_string(map_size.columns)) | bold | color(Color::CyanLight) | hcenter,
        separatorEmpty(),
        hbox(std::move(pulse)) | hcenter,
        text(spinner[(tick / 2) % spinner.size()]) | color(Color::GreenLight) | hcenter,
        text("PLEASE WAIT...") | color(Color::GreenLight) | hcenter,
        separatorEmpty(),
        separator() | color(Color::Cyan),
        text("Q: quit (waits for current batch)") | color(Color::GrayLight) | hcenter,
    }) | size(WIDTH, EQUAL, 44) | borderRounded | color(Color::CyanLight) |
        center | bgcolor(Color::Black);
}
