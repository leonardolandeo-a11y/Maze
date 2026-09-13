#include "circuit_escape/DeathAnimation.h"
#include "circuit_escape/UI_Layout.h"

#include <string>

ftxui::Element DeathAnimation::RenderSystemFailure(int frame, EndReason reason) const{
    ftxui::Elements deathLines;

    std::string reasonText;

    if (reason == EndReason::noEnergy) {
        reasonText = "ENERGY DEPLETED";
    }
    else if (reason == EndReason::turnLimit) {
        reasonText = "TURN LIMIT EXCEEDED";
    }
    else {
        reasonText = "UNKNOWN SYSTEM FAILURE";
    }

    if (frame >= 0) {
        deathLines.push_back(
            ftxui::text("> CRITICAL SYSTEM FAILURE") |
            ftxui::color(ftxui::Color::RedLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    if (frame >= 8) {
        deathLines.push_back(
            ftxui::text("> CONNECTION LOST") |
            ftxui::color(ftxui::Color::Red) |
            ftxui::center
        );
    }

    if (frame >= 16) {
        deathLines.push_back(ftxui::text(""));

        deathLines.push_back(
            ftxui::text("GAME OVER") |
            ftxui::color(ftxui::Color::RedLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    if (frame >= 24) {
        deathLines.push_back(ftxui::text(""));

        deathLines.push_back(
            ftxui::text(reasonText) |
            ftxui::color(ftxui::Color::White) |
            ftxui::bold |
            ftxui::center
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(deathLines) |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, gameViewWidth),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element DeathAnimation::RenderDeathFrame(int frame, EndReason reason) const{
    return RenderSystemFailure(frame, reason);
}