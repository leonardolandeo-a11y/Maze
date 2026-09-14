#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/UI_Layout.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include <ftxui/screen/color.hpp>

namespace {
    constexpr int canvasWidth = 90;
    constexpr int canvasHeight = 40;
    constexpr int groundPixelRow = 32;

    using PixelCanvas = std::vector<std::string>;

    const std::vector<std::string> agentSideWalk1 = {
        "....HHH.......",
        "...HHHH.......",
        "...HFFF.......",
        "...HFE........",
        "...HFFF.......",
        "....HH........",
        "...TTTT.......",
        "..TTTTTT......",
        "...TTT........",
        "..L.TT........",
        ".LL..L........",
        ".....LL......."
    };

    const std::vector<std::string> agentSideWalk2 = {
        "....HHH.......",
        "...HHHH.......",
        "...HFFF.......",
        "...HFE........",
        "...HFFF.......",
        "....HH........",
        "...TTTT.......",
        "..TTTTTT......",
        "...TTT........",
        "....TTL.......",
        "...LL..L......",
        "..LL...LL....."
    };

    const std::vector<std::string> agentBack = {
        "...HHHH.......",
        "..HHHHHH......",
        "..HBBBBH......",
        "..HBBBBH......",
        "..HHHHHH......",
        "...HHHH.......",
        "...TTTT.......",
        "..TTTTTT......",
        "..T.TT.T......",
        "...TTTT.......",
        "...L..L.......",
        "..LL..LL......"
    };

    const std::vector<std::string> agentSide = {
        "....HHH.......",
        "...HHHH.......",
        "...HFFF.......",
        "...HFE........",
        "...HFFF.......",
        "....HH........",
        "...TTTT.......",
        "..TTTTT.......",
        "...TTT........",
        "...TTT........",
        "...L.L........",
        "..LL.LL......."
    };

    const std::vector<std::string> agentFront = {
        "...HHHH.......",
        "..HHHHHH......",
        "..HFFFFH......",
        "..HFEEFH......",
        "..HFFFFH......",
        "...HHHH.......",
        "...TTTT.......",
        "..TTTTTT......",
        "..T.TT.T......",
        "...TTTT.......",
        "...L..L.......",
        "..LL..LL......"
    };

    const std::vector<std::string> agentArmHalfRaised = {
        "...HHHH.........",
        "..HHHHHH........",
        "..HFFFFH........",
        "..HFEEFH........",
        "..HFFFFH........",
        "...HHHH.........",
        "...TTTT......T..",
        "..TTTTTT....TT..",
        "..T.TT.T...TT...",
        "...TTTT.........",
        "...L..L.........",
        "..LL..LL........"
    };

    const std::vector<std::string> agentArmRaised = {
        ".............TT.",
        "............TT..",
        "...HHHH.....T...",
        "..HHHHHH....T...",
        "..HFFFFH....T...",
        "..HFEEFH....T...",
        "..HFFFFH...TT...",
        "...HHHH...TT....",
        "...TTTT.........",
        "..TTTTTT........",
        "...L..L.........",
        "..LL..LL........"
    };

    const std::vector<std::string> agentDanceLeft = {
        "..TT...........",
        "...T.HHHH......",
        "...THHHHHH.....",
        "...THFFFFH.....",
        "...THFEEFH.....",
        "...THFFFFH.....",
        "....HHHH...T...",
        "....TTTT..TT...",
        "...TTTTTTT.....",
        "....TTTT.......",
        "...LL..L.......",
        "..LL....LL....."
    };

    const std::vector<std::string> agentDanceRight = {
        "...........TT..",
        "......HHHH.T...",
        ".....HHHHHHT...",
        ".....HFFFFHT...",
        ".....HFEEFHT...",
        ".....HFFFFHT...",
        "...T..HHHH.....",
        "...TT.TTTT.....",
        ".....TTTTTTT...",
        ".......TTTT....",
        ".......L..LL...",
        "......LL....LL."
    };

    ftxui::Color PixelColor(char pixel){
        using namespace ftxui;

        switch (pixel){
            case 'H':
                return Color::RGB(185, 195, 205);

            case 'B':
                return Color::RGB(90, 100, 115);

            case 'F':
                return Color::RGB(255, 205, 165);

            case 'E':
                return Color::Black;

            case 'T':
                return Color::BlueLight;

            case 'L':
                return Color::RGB(80, 90, 105);

            case 'G':
                return Color::RGB(65, 65, 70);

            case 'P':
                return Color::GreenLight;

            case 'Y':
                return Color::YellowLight;

            case 'C':
                return Color::CyanLight;

            case 'W':
                return Color::White;

            default:
                return Color::Black;
        }
    }

    PixelCanvas CreateCanvas(){
        return PixelCanvas(
            canvasHeight,
            std::string(canvasWidth, '.')
        );
    }

    int SpriteWidth(const std::vector<std::string>& sprite){
        std::size_t width = 0;

        for (const auto& row : sprite){
            width = std::max(
                width,
                row.size()
            );
        }

        return static_cast<int>(width);
    }

    void DrawSprite(
        PixelCanvas& canvas,
        const std::vector<std::string>& sprite,
        int top,
        int left
    ){
        for (int row = 0; row < static_cast<int>(sprite.size()); ++row){
            const int canvasRow =
                top + row;

            if (
                canvasRow < 0 ||
                canvasRow >= canvasHeight
            ){
                continue;
            }

            for (int col = 0; col < static_cast<int>(sprite[row].size()); ++col){
                const int canvasCol =
                    left + col;

                if (
                    canvasCol < 0 ||
                    canvasCol >= canvasWidth
                ){
                    continue;
                }

                if (sprite[row][col] == '.'){
                    continue;
                }

                canvas[canvasRow][canvasCol] =
                    sprite[row][col];
            }
        }
    }

    void DrawSpriteCentered(
        PixelCanvas& canvas,
        const std::vector<std::string>& sprite,
        int top
    ){
        const int left =
            (
                canvasWidth -
                SpriteWidth(sprite)
            ) / 2;

        DrawSprite(
            canvas,
            sprite,
            top,
            left
        );
    }

    void DrawGround(PixelCanvas& canvas){
        for (int col = 8; col < canvasWidth - 8; ++col){
            canvas[groundPixelRow][col] =
                'G';
        }
    }

    int BottomAlignedTop(
        const std::vector<std::string>& sprite
    ){
        return groundPixelRow -
            static_cast<int>(sprite.size());
    }

    ftxui::Element RenderPixelPair(
        char topPixel,
        char bottomPixel
    ){
        using namespace ftxui;

        if (
            topPixel == '.' &&
            bottomPixel == '.'
        ){
            return text(" ");
        }

        if (
            topPixel != '.' &&
            bottomPixel == '.'
        ){
            return text("▀") |
                color(
                    PixelColor(topPixel)
                );
        }

        if (
            topPixel == '.' &&
            bottomPixel != '.'
        ){
            return text("▄") |
                color(
                    PixelColor(bottomPixel)
                );
        }

        if (topPixel == bottomPixel){
            return text("█") |
                color(
                    PixelColor(topPixel)
                );
        }

        return text("▀") |
            color(
                PixelColor(topPixel)
            ) |
            bgcolor(
                PixelColor(bottomPixel)
            );
    }

    ftxui::Element RenderCanvas(
        const PixelCanvas& canvas
    ){
        using namespace ftxui;

        Elements rows;

        for (int row = 0; row < canvasHeight; row += 2){
            Elements columns;

            for (int col = 0; col < canvasWidth; ++col){
                columns.push_back(
                    RenderPixelPair(
                        canvas[row][col],
                        canvas[row + 1][col]
                    )
                );
            }

            rows.push_back(
                hbox(columns)
            );
        }

        return UILayout::CenterGameView(
            vbox(rows) |
            bgcolor(Color::Black)
        );
    }

    void DrawCelebrationParticles(
        PixelCanvas& canvas,
        int frame
    ){
        struct Particle{
            int x;
            int offset;
            char pixel;
        };

        const std::array<Particle, 28> particles = {{
            {10,  0, 'P'},
            {14,  7, 'Y'},
            {18, 13, 'C'},
            {22,  4, 'P'},
            {26, 18, 'Y'},
            {30, 10, 'C'},
            {34, 22, 'P'},
            {38,  2, 'Y'},
            {42, 16, 'C'},
            {46,  8, 'P'},
            {50, 25, 'Y'},
            {54,  5, 'C'},
            {58, 20, 'P'},
            {62, 12, 'Y'},
            {66, 27, 'C'},
            {70,  3, 'P'},
            {74, 15, 'Y'},
            {78, 23, 'C'},

            {12, 19, 'Y'},
            {20, 26, 'P'},
            {28,  1, 'C'},
            {36, 11, 'Y'},
            {44, 21, 'P'},
            {52, 14, 'C'},
            {60,  6, 'Y'},
            {68, 24, 'P'},
            {76,  9, 'C'},
            {80, 17, 'Y'}
        }};

        for (const auto& particle : particles){
            const int y =
                (
                    frame +
                    particle.offset
                ) %
                (groundPixelRow - 2);

            if (
                y >= 0 &&
                y < groundPixelRow - 1
            ){
                canvas[y][particle.x] =
                    particle.pixel;
            }
        }
    }
}

ftxui::Element VictoryAnimation::RenderWalkingSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const auto& walkSprite =
        (
            frame / 3
        ) % 2 == 0
            ? agentSideWalk1
            : agentSideWalk2;

    const int spriteWidth =
        SpriteWidth(
            walkSprite
        );

    const int startX = 8;

    const int centerX =
        (
            canvasWidth -
            spriteWidth
        ) / 2;

    const int x =
        startX +
        (
            (centerX - startX) *
            frame
        ) /
        (walkEndFrame - 1);

    DrawSprite(
        canvas,
        walkSprite,
        BottomAlignedTop(
            walkSprite
        ),
        x
    );

    return RenderCanvas(canvas);
}

ftxui::Element VictoryAnimation::RenderTurnSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int turnFrame =
        frame - walkEndFrame;

    if (turnFrame < 3){
        DrawSpriteCentered(
            canvas,
            agentBack,
            BottomAlignedTop(
                agentBack
            )
        );
    }
    else if (turnFrame < 6){
        DrawSpriteCentered(
            canvas,
            agentSide,
            BottomAlignedTop(
                agentSide
            )
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            agentFront,
            BottomAlignedTop(
                agentFront
            )
        );
    }

    return RenderCanvas(canvas);
}

ftxui::Element VictoryAnimation::RenderRaiseArmSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int raiseFrame =
        frame - turnEndFrame;

    if (raiseFrame < 4){
        DrawSpriteCentered(
            canvas,
            agentFront,
            BottomAlignedTop(
                agentFront
            )
        );
    }
    else if (raiseFrame < 8){
        DrawSpriteCentered(
            canvas,
            agentArmHalfRaised,
            BottomAlignedTop(
                agentArmHalfRaised
            )
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            agentArmRaised,
            BottomAlignedTop(
                agentArmRaised
            )
        );
    }

    return RenderCanvas(canvas);
}

ftxui::Element VictoryAnimation::RenderCelebrationSequence(int frame) const{
    PixelCanvas canvas = CreateCanvas();

    DrawGround(canvas);
    DrawCelebrationParticles(canvas,frame - raiseArmEndFrame);

    const int danceFrame = frame - raiseArmEndFrame;

    if ((danceFrame / 4) % 2 == 0){
        DrawSpriteCentered(
            canvas,
            agentDanceLeft,
            BottomAlignedTop(agentDanceLeft)
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            agentDanceRight,
            BottomAlignedTop(agentDanceRight)
        );
    }

    return RenderCanvas(canvas);
}

ftxui::Element VictoryAnimation::RenderVictoryScreen(int frame) const{
    const std::array<std::string, 6> victoryTitle = {
        "██╗   ██╗██╗ ██████╗████████╗ ██████╗ ██████╗ ██╗   ██╗",
        "██║   ██║██║██╔════╝╚══██╔══╝██╔═══██╗██╔══██╗╚██╗ ██╔╝",
        "██║   ██║██║██║        ██║   ██║   ██║██████╔╝ ╚████╔╝ ",
        "╚██╗ ██╔╝██║██║        ██║   ██║   ██║██╔══██╗  ╚██╔╝  ",
        " ╚████╔╝ ██║╚██████╗   ██║   ╚██████╔╝██║  ██║   ██║   ",
        "  ╚═══╝  ╚═╝ ╚═════╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   "
    };

    ftxui::Elements titleLines;

    for (const auto& line : victoryTitle){
        titleLines.push_back(
            ftxui::text(line) |
            ftxui::color(
                ftxui::Color::GreenLight
            ) |
            ftxui::bold |
            ftxui::center
        );
    }

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text(
            "ESCAPE PROTOCOL COMPLETE"
        ) |
        ftxui::color(
            ftxui::Color::White
        ) |
        ftxui::bold |
        ftxui::center
    );

    if (frame >= maxFrames - 1){
        titleLines.push_back(
            ftxui::text("")
        );

        titleLines.push_back(
            ftxui::text(
                "ENTER - PLAY AGAIN    Q - EXIT"
            ) |
            ftxui::color(
                ftxui::Color::GrayLight
            ) |
            ftxui::center
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(
                titleLines
            ),
            ftxui::filler()
        }) |
        ftxui::bgcolor(
            ftxui::Color::Black
        )
    );
}

ftxui::Element VictoryAnimation::RenderVictoryFrame(int frame) const{
    if (frame < walkEndFrame){
        return RenderWalkingSequence(
            frame
        );
    }

    if (frame < turnEndFrame){
        return RenderTurnSequence(
            frame
        );
    }

    if (frame < raiseArmEndFrame){
        return RenderRaiseArmSequence(
            frame
        );
    }

    if (frame < celebrationEndFrame){
        return RenderCelebrationSequence(
            frame
        );
    }

    return RenderVictoryScreen(
        frame
    );
}