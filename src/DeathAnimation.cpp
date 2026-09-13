#include "circuit_escape/DeathAnimation.h"
#include "circuit_escape/UI_Layout.h"

#include <array>
#include <string>
#include <vector>

ftxui::Element DeathAnimation::RenderGlitchSequence(int frame) const{
    const std::string glitchSymbols =
        "01#@%&*+<>[]{}|/\\?X$!";

    std::vector<std::string> screenMatrix(
        gameViewHeight,
        std::string(gameViewWidth, '0')
    );

    for (int row = 0; row < gameViewHeight; ++row) {
        for (int col = 0; col < gameViewWidth; ++col) {
            const std::size_t symbolIndex = (
                row * 37 +
                col * 19 +
                frame * 17 +
                row * col
            ) % glitchSymbols.size();

            screenMatrix[row][col] =
                glitchSymbols[symbolIndex];
        }
    }

    ftxui::Elements glitchLines;

    for (int row = 0; row < gameViewHeight; ++row) {
        ftxui::Elements rowElements;

        for (int col = 0; col < gameViewWidth; ++col) {
            const std::string character(
                1,
                screenMatrix[row][col]
            );

            const int interferencePattern = (
                row * 11 +
                col * 7 +
                frame * 5
            ) % 13;

            if (interferencePattern == 0) {
                rowElements.push_back(
                    ftxui::text(character) |
                    ftxui::color(ftxui::Color::White) |
                    ftxui::bold
                );
            }
            else if (interferencePattern <= 3) {
                rowElements.push_back(
                    ftxui::text(character) |
                    ftxui::color(ftxui::Color::RedLight) |
                    ftxui::bold
                );
            }
            else {
                rowElements.push_back(
                    ftxui::text(character) |
                    ftxui::color(ftxui::Color::Red)
                );
            }
        }

        glitchLines.push_back(
            ftxui::hbox(rowElements)
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox(glitchLines) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element DeathAnimation::RenderBlankTransition() const{
    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::text(""),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element DeathAnimation::RenderSystemFailure(int frame, EndReason reason) const{
    ftxui::Elements failureLines;

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

    const int failureFrame =
        frame - blankEndFrame;

    if (failureFrame >= 0) {
        failureLines.push_back(
            ftxui::text("> CRITICAL SYSTEM FAILURE") |
            ftxui::color(ftxui::Color::RedLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    if (failureFrame >= 5) {
        failureLines.push_back(
            ftxui::text("> CONNECTION LOST") |
            ftxui::color(ftxui::Color::Red) |
            ftxui::center
        );
    }

    if (failureFrame >= 10) {
        failureLines.push_back(
            ftxui::text("> " + reasonText) |
            ftxui::color(ftxui::Color::White) |
            ftxui::bold |
            ftxui::center
        );
    }

    if (failureFrame >= 13) {
        const std::string cursor =
            frame % 2 == 0 ? "_" : " ";

        failureLines.push_back(
            ftxui::text(cursor) |
            ftxui::color(ftxui::Color::RedLight) |
            ftxui::center
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(failureLines) |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, gameViewWidth),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element DeathAnimation::RenderGameOver(EndReason reason) const{
    const std::array<std::string, 6> gameOverTitle = {
        " ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ",
        "██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗",
        "██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝",
        "██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗",
        "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║",
        " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝"
    };

    std::string reasonText;

    if (reason == EndReason::noEnergy) {
        reasonText = "ENERGY DEPLETED";
    }
    else if (reason == EndReason::turnLimit) {
        reasonText = "TURN LIMIT EXCEEDED";
    }
    else {
        reasonText = "SYSTEM FAILURE";
    }

    ftxui::Elements titleLines;

    for (const auto& line : gameOverTitle) {
        titleLines.push_back(
            ftxui::text(line) |
            ftxui::color(ftxui::Color::RedLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text(reasonText) |
        ftxui::color(ftxui::Color::White) |
        ftxui::bold |
        ftxui::center
    );

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text("ESCAPE PROTOCOL TERMINATED") |
        ftxui::color(ftxui::Color::Red) |
        ftxui::center
    );

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(titleLines) |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, gameViewWidth),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element DeathAnimation::RenderDeathFrame(int frame, EndReason reason) const{
    if (frame < glitchEndFrame) {
        return RenderGlitchSequence(frame);
    }

    if (frame < blankEndFrame) {
        return RenderBlankTransition();
    }

    if (frame < failureEndFrame) {
        return RenderSystemFailure(frame, reason);
    }

    return RenderGameOver(reason);
}