#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/DeathAnimation.h"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <cassert>
#include <string>

namespace {
std::string render_animation(ftxui::Element element) {
    auto screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(180),
        ftxui::Dimension::Fixed(65)
    );
    ftxui::Render(screen, std::move(element));
    return screen.ToString();
}
}

void run_animation_tests() {
    Observation observation{};
    observation.energy = 12;
    observation.maximumEnergy = 40;
    observation.score = 7;
    observation.turn = 6;
    observation.collectedResources = 2;

    StartupAnimation startup;
    const std::string boot = render_animation(startup.RenderStartupFrame(0));
    const std::string ready = render_animation(
        startup.RenderStartupFrame(StartupAnimation::maxFrames - 1));
    assert(!boot.empty());
    assert(ready.find("SYSTEM READY") != std::string::npos);

    VictoryAnimation victory;
    const std::string early_victory = render_animation(
        victory.RenderVictoryFrame(VictoryAnimation::maxFrames - 2, observation));
    const std::string final_victory = render_animation(
        victory.RenderVictoryFrame(VictoryAnimation::maxFrames - 1, observation));
    assert(early_victory.find("ENTER - MAIN MENU") == std::string::npos);
    assert(final_victory.find("ENTER - MAIN MENU") != std::string::npos);
    assert(final_victory.find("Q - EXIT") != std::string::npos);
    assert(final_victory.find("PLAY AGAIN") == std::string::npos);

    DeathAnimation death;
    const std::string early_death = render_animation(
        death.RenderDeathFrame(DeathAnimation::maxFrames - 2, EndReason::captured, observation));
    const std::string final_death = render_animation(
        death.RenderDeathFrame(DeathAnimation::maxFrames - 1, EndReason::captured, observation));
    assert(early_death.find("ENTER - MAIN MENU") == std::string::npos);
    assert(final_death.find("ENTER - MAIN MENU") != std::string::npos);
    assert(final_death.find("Q - EXIT") != std::string::npos);
    assert(final_death.find("CAPTURED BY THE GUARD") != std::string::npos);

    const std::string no_energy = render_animation(
        death.RenderDeathFrame(DeathAnimation::maxFrames - 1, EndReason::noEnergy, observation));
    const std::string turn_limit = render_animation(
        death.RenderDeathFrame(DeathAnimation::maxFrames - 1, EndReason::turnLimit, observation));
    assert(no_energy.find("ENERGY DEPLETED") != std::string::npos);
    assert(turn_limit.find("TURN LIMIT EXCEEDED") != std::string::npos);
}
