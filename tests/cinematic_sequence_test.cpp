#include "circuit_escape/animation_sequence.h"

#include <ftxui/screen/screen.hpp>
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {
std::string render(ftxui::Element element, ftxui::Dimensions viewport,
                   const std::string& name = "") {
    element->ComputeRequirement();
    assert(element->requirement().min_x <= viewport.dimx);
    assert(element->requirement().min_y <= viewport.dimy);
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(viewport.dimx),
                                       ftxui::Dimension::Fixed(viewport.dimy));
    ftxui::Render(screen, element);
    std::string plain;
    for (int y = 0; y < viewport.dimy; ++y) {
        for (int x = 0; x < viewport.dimx; ++x) {
            const auto& character = screen.PixelAt(x, y).character;
            plain += character.empty() ? " " : character;
        }
        plain += '\n';
    }
    if (const char* directory = std::getenv("MAZE_UI_PREVIEW_DIR"); directory && !name.empty()) {
        std::filesystem::create_directories(directory);
        const auto path = std::filesystem::path(directory) /
            ("cinematic-" + name + "-" + std::to_string(viewport.dimx) + "x" + std::to_string(viewport.dimy));
        std::ofstream(path.string() + ".txt") << plain;
        std::ofstream(path.string() + ".ansi") << screen.ToString();
    }
    return plain;
}
}

void run_cinematic_sequence_tests() {
    // Counts come from the original dispatch boundaries, not the old results' duration.
    static_assert(CinematicStartupAnimation::maxFrames == 166);
    static_assert(CinematicVictoryAnimation::maxFrames == 78);
    static_assert(CinematicDeathAnimation::maxFrames == 142);
    assert(!AnimationSequence::startupComplete(165));
    assert(AnimationSequence::startupComplete(166));
    static_assert(AnimationSequence::startupFrames == 166);
    assert(!AnimationSequence::victoryComplete(112));
    assert(AnimationSequence::victoryComplete(113));
    assert(!AnimationSequence::deathComplete(176));
    assert(AnimationSequence::deathComplete(177));

    Observation observation{};
    observation.energy = 12;
    observation.maximumEnergy = 40;
    observation.turn = 6;
    observation.score = 7;
    observation.collectedResources = 2;

    for (auto viewport : {ftxui::Dimensions{80,24}, {100,30}, {120,35}, {160,45}}) {
        auto sprite = CinematicVictoryAnimation{}.RenderFrame(46, viewport);
        sprite->ComputeRequirement();
        assert((sprite->requirement().min_x > 90) == (viewport.dimx > 90));
        if (viewport.dimx > 90) {
            std::array<std::set<std::string>, 2> palettes;
            const std::array sizes{ftxui::Dimensions{90,20}, viewport};
            for (int i = 0; i < 2; ++i) {
                const auto size = sizes[i];
                auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(size.dimx),
                                                    ftxui::Dimension::Fixed(size.dimy));
                ftxui::Render(screen, CinematicVictoryAnimation{}.RenderFrame(46, size));
                for (int y = 0; y < size.dimy; ++y)
                    for (int x = 0; x < size.dimx; ++x) {
                        const auto& pixel = screen.PixelAt(x, y);
                        palettes[i].insert(pixel.background_color.Print(false));
                        if (pixel.character == "█" || pixel.character == "▀" || pixel.character == "▄")
                            palettes[i].insert(pixel.foreground_color.Print(false));
                    }
            }
            assert(palettes[0] == palettes[1]);
        }
        // Both sides of every original phase boundary, including falling debris and rest.
        for (int frame : {0, 4, 37, 38, 41, 42, 63, 64, 93, 94, 109, 110, 123, 124, 155, 156, 165}) {
            const auto text = render(AnimationSequence::startup(frame, viewport), viewport,
                                     "startup-" + std::to_string(frame));
            assert(text.find("WELCOME") == std::string::npos);
        }
        for (int frame : {0, 23, 24, 33, 34, 45, 46, 77}) {
            const auto text = render(AnimationSequence::victory(frame, observation, viewport), viewport,
                                     "victory-" + std::to_string(frame));
            assert(text.find("RUN SUMMARY") == std::string::npos);
            assert(text.find("FINAL STATS") == std::string::npos);
            assert(text.find("[ ENTER ]") == std::string::npos);
            assert(text.find("█") != std::string::npos || text.find("▀") != std::string::npos);
        }
        for (int frame : {0, 15, 16, 27, 28, 41, 42, 49, 50, 61, 62, 71, 72, 83, 84, 93, 94, 125, 126, 141}) {
            const auto text = render(AnimationSequence::death(frame, EndReason::captured, observation, viewport),
                                     viewport, "death-" + std::to_string(frame));
            assert(text.find("GAME OVER") == std::string::npos);
            assert(text.find("RUN SUMMARY") == std::string::npos);
            assert(text.find("[ ENTER ]") == std::string::npos);
        }
        const auto victory = render(AnimationSequence::victory(78, observation, viewport), viewport);
        assert(victory.find("ESCAPE COMPLETED") != std::string::npos);
        assert(victory.find("¡Has escapado del circuito!") != std::string::npos);
        assert(victory.find("[ ENTER ]") == std::string::npos);
        const auto finished = render(AnimationSequence::victory(113, observation, viewport), viewport, "victory-panel");
        auto stats = finished;
        std::erase(stats, ' ');
        for (const auto* label : {"Energía:12/40", "Puntaje:7", "Turnos:6", "Recursos:2"})
            assert(stats.find(label) != std::string::npos);
        assert(finished.find("[ ENTER ]") != std::string::npos);
        assert(finished.find("███████╗███████╗ ██████╗ █████╗ ██████╗ ███████╗") != std::string::npos);
        assert(finished.find("██╔════╝██╔═══██╗████╗ ████║██╔══██╗██║     ██╔════╝╚══██╔══╝██╔════╝██╔══██╗") != std::string::npos);
        assert(finished.find("╰─┤█├─╯") == std::string::npos);
        assert(finished.find("MAZE") == std::string::npos);
        assert(finished.find("CIRCUITO DE ESCAPE") == std::string::npos);
        assert(render(AnimationSequence::victory(112, observation, viewport), viewport)
                   .find("[ ENTER ]") == std::string::npos);
        for (auto reason : {EndReason::captured, EndReason::noEnergy, EndReason::turnLimit}) {
            const auto first = render(AnimationSequence::death(142, reason, observation, viewport), viewport);
            assert(first.find("GAME OVER") != std::string::npos);
            assert(first.find("[ ENTER ]") == std::string::npos);
            const auto final = render(AnimationSequence::death(177, reason, observation, viewport), viewport, "death-panel");
            assert(final.find("[ ENTER ]") != std::string::npos);
            stats = final;
            std::erase(stats, ' ');
            for (const auto* stat : {"Energía:12/40", "Puntaje:7", "Turnos:6", "Recursos:2"})
                assert(stats.find(stat) != std::string::npos);
            assert(final.find("██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗") != std::string::npos);
            assert(final.find("MAZE") == std::string::npos);
            assert(final.find("CIRCUITO DE ESCAPE") == std::string::npos);
            assert(final.find(reason == EndReason::captured ? "CAPTURED BY THE GUARD" :
                reason == EndReason::noEnergy ? "ENERGY DEPLETED" : "TURN LIMIT EXCEEDED") != std::string::npos);
        }
    }
    const ftxui::Dimensions original{90,20};
    assert(render(CinematicStartupAnimation{}.RenderFrame(29, original), original)
        .find("SYSTEM LINK ESTABLISHED") != std::string::npos);
    assert(render(CinematicStartupAnimation{}.RenderFrame(165, original), original)
        .find("SYSTEM READY") != std::string::npos);
}
