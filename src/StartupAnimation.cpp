#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/UI_Layout.h"
#include <vector>

ftxui::Element StartupAnimation::RenderBootSquence(int frame) const{
    ftxui::Elements bootLines;

    if (frame >= 4) {
        bootLines.push_back(
            ftxui::text("> INITIALIZING CORE...") |
            ftxui::color(ftxui::Color::GreenLight)
        );
    }

    if (frame >= 9) {
        bootLines.push_back(
            ftxui::text("> NAVIGATION SYSTEM ........ OK") |
            ftxui::color(ftxui::Color::GreenLight)
        );
    }

    if (frame >= 14) {
        bootLines.push_back(
            ftxui::text("> GRID MATRIX .............. OK") |
            ftxui::color(ftxui::Color::GreenLight)
        );
    }

    if (frame >= 19) {
        bootLines.push_back(
            ftxui::text("> MEMORY CHECK ............. OK") |
            ftxui::color(ftxui::Color::GreenLight)
        );
    }

    if (frame >= 24) {
        bootLines.push_back(
            ftxui::text("> ESCAPE PROTOCOL .......... ACTIVE") |
            ftxui::color(ftxui::Color::CyanLight)
        );
    }

    if (frame >= 29) {
        bootLines.push_back(ftxui::text(""));

        bootLines.push_back(
            ftxui::text("> SYSTEM LINK ESTABLISHED") |
            ftxui::color(ftxui::Color::CyanLight) |
            ftxui::bold
        );
    }

    if (frame >= 4) {
        const std::string cursor = frame % 2 == 0 ? "█" : " ";

        bootLines.push_back(
            ftxui::text(cursor) |
            ftxui::color(ftxui::Color::GreenLight)
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::hbox({
                ftxui::filler(),
                ftxui::vbox(bootLines),
                ftxui::filler()
            }),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element StartupAnimation::RenderBlankTransition(int frame) const{
    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::text(""),
            ftxui::filler()
        }) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}


ftxui::Element StartupAnimation::RenderLogoFormation(int frame) const{

    const int glitchFrame = frame - glitchStartFrame;

    const std::string backgroundSymbols =
        "01#@%&*+<>[]{}|/\\?X$";

    const std::string logoSymbols =
        "MAZEGRIDEXITROUTEENERGYTRAP";

    const std::array<std::string, 6> logoMask = {
        "XXXX   XXXX XXXXXX XXXXXXXXXXXXXXXX",
        "XXXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
        "XXXXXXXXXXXXXXXXXXX  XXXXX XXXXXX  ",
        "XXXXXXXXXXXXXXXXXXX XXXXX  XXXXXX  ",
        "XXX XXX XXXXXX  XXXXXXXXXXXXXXXXXXX",
        "XXX     XXXXXX  XXXXXXXXXXXXXXXXXXX"
    };
    std::vector<std::string> screenMatrix(
        gameViewHeight,
        std::string(gameViewWidth, '0')
    );

    std::vector<std::string> logoActive(
        gameViewHeight,
        std::string(gameViewWidth, '0')
    );

    for (int row = 0; row < gameViewHeight; ++row) {
        for (int col = 0; col < gameViewWidth; ++col) {
            const std::size_t symbolIndex = (
                row * 31 +
                col * 17 +
                glitchFrame * 13 +
                row * col
            ) % backgroundSymbols.size();

            screenMatrix[row][col] =
                backgroundSymbols[symbolIndex];
        }
    }

    const int logoHeight =
        static_cast<int>(logoMask.size());

    const int logoWidth =
        static_cast<int>(logoMask[0].size());

    const int logoStartRow =
        (gameViewHeight - logoHeight) / 2;

    const int logoStartCol =
        (gameViewWidth - logoWidth) / 2;

    if (frame >= logoFormationStartFrame) {
        int formationProgress =
            (
                (frame - logoFormationStartFrame) *
                100
            ) /
            (
                titleRevealFrame -
                logoFormationStartFrame
            );

        formationProgress =
            std::clamp(formationProgress, 0, 100);

        for (int row = 0; row < logoHeight; ++row) {
            for (int col = 0; col < logoWidth; ++col) {
                if (logoMask[row][col] != 'X') {
                    continue;
                }

                const int cellThreshold = (
                    row * 23 +
                    col * 11
                ) % 100;

                if (formationProgress < cellThreshold) {
                    continue;
                }

                const int screenRow =
                    logoStartRow + row;

                const int screenCol =
                    logoStartCol + col;

                const std::size_t logoSymbolIndex = (
                    row * 13 +
                    col * 7 +
                    glitchFrame
                ) % logoSymbols.size();

                screenMatrix[screenRow][screenCol] =
                    logoSymbols[logoSymbolIndex];

                logoActive[screenRow][screenCol] =
                    '1';
            }
        }
    }

    ftxui::Elements interferenceLines;

    for (int row = 0; row < gameViewHeight; ++row) {
        ftxui::Elements rowElements;

        for (int col = 0; col < gameViewWidth; ++col) {
            const std::string character(
                1,
                screenMatrix[row][col]
            );

            if (logoActive[row][col] == '1') {
                rowElements.push_back(
                    ftxui::text(character) |
                    ftxui::color(ftxui::Color::CyanLight) |
                    ftxui::bold
                );
            }
            else {
                rowElements.push_back(
                    ftxui::text(character) |
                    ftxui::color(ftxui::Color::GreenLight)
                );
            }
        }

        interferenceLines.push_back(
            ftxui::hbox(rowElements)
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox(interferenceLines) |
        ftxui::bgcolor(ftxui::Color::Black)
    );
}

ftxui::Element StartupAnimation::RenderTitleSequence(int frame) const{

    //Titulo MAZE ya completamente formado
    const std::array<std::string, 6> mazeTitle = {
        "███╗   ███╗ █████╗ ███████╗███████╗",
        "████╗ ████║██╔══██╗╚══███╔╝██╔════╝",
        "██╔████╔██║███████║  ███╔╝ █████╗  ",
        "██║╚██╔╝██║██╔══██║ ███╔╝  ██╔══╝  ",
        "██║ ╚═╝ ██║██║  ██║███████╗███████╗",
        "╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝"
    };

    ftxui::Elements titleLines;

    for (const auto& line : mazeTitle) {
        titleLines.push_back(
            ftxui::text(line) |
            ftxui::color(ftxui::Color::CyanLight) |
            ftxui::bold |
            ftxui::center
        );
    }
    //Primero el titulo se queda solo
    if (frame >= subtitleFrame) {
        titleLines.push_back(ftxui::text(""));

        titleLines.push_back(
            ftxui::text("ESCAPE PROTOCOL ACTIVE") |
            ftxui::color(ftxui::Color::GreenLight) |
            ftxui::bold |
            ftxui::center
        );
    }

    //Luego aparece la carga
    if (frame >= loadingStartFrame) {
        int loadingProgress =((frame - loadingStartFrame) *100) /(loadingCompleteFrame -loadingStartFrame);

        loadingProgress =std::clamp(loadingProgress, 0, 100);

        std::string loadingMessage;

        if (loadingProgress < 25) {
            loadingMessage ="LOADING GRID...";
        }
        else if (loadingProgress < 50) {
            loadingMessage ="LOADING NAVIGATION SYSTEM...";
        }
        else if (loadingProgress < 75) {
            loadingMessage ="LOADING ESCAPE PROTOCOL...";
        }
        else if (loadingProgress < 100) {
            loadingMessage ="FINALIZING SYSTEM...";
        }
        else {
            loadingMessage ="SYSTEM READY";
        }

        constexpr int barWidth = 36;

        const int filledWidth =(loadingProgress * barWidth) / 100;

        std::string loadingBar = "[";

        for (int i = 0; i < barWidth; ++i) {
            if (i < filledWidth) {
                loadingBar += "█";
            }
            else {
                loadingBar += "░";
            }
        }

        loadingBar += "] ";
        loadingBar += std::to_string(loadingProgress);
        loadingBar += "%";

        titleLines.push_back(ftxui::text(""));

        titleLines.push_back(
            ftxui::text(loadingMessage) |
            ftxui::color(ftxui::Color::CyanLight) |
            ftxui::center
        );

        titleLines.push_back(
            ftxui::text(loadingBar) |
            ftxui::color(
                loadingProgress == 100
                    ? ftxui::Color::GreenLight
                    : ftxui::Color::CyanLight
            ) |
            ftxui::bold |
            ftxui::center
        );
    }

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


ftxui::Element StartupAnimation::RenderStartupFrame(int frame) const{

    //Secuencia inicial tipo sistema
    if (frame < bootEndFrame) {
        return RenderBootSquence(frame);
    }

    if (frame < glitchStartFrame) {
        return RenderBlankTransition(frame);
    }

    //Caos cinematografico que termina formando MAZE
    if (frame < titleRevealFrame) {
        return RenderLogoFormation(frame);
    }
    return RenderTitleSequence(frame);

}