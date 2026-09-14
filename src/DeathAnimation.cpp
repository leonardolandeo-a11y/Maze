#include "circuit_escape/DeathAnimation.h"
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

    const std::vector<std::string> agentFallingBack = {
        "........................",
        "...LL....TTTTTT...HHHH..",
        "..LLLL..TTTTTTTT.HHHHHH.",
        "...LL...TTTTTTTT.HBBBBH.",
        "........TTTTTTTT.HBBBBH.",
        ".........TTTTTT..HHHHHH.",
        "..................HHHH..",
        "........................"
    };

    const std::vector<std::string> agentLyingBack = {
        "..........................",
        "..LLL...TTTTTTT....HHHH...",
        ".LLLL..TTTTTTTTT..HHHHHH..",
        "..LLL...TTTTTTT...HBBBBH..",
        "........TTTTTTT...HBBBBH..",
        "...................HHHH....",
        ".........................."
    };

    const std::vector<std::string> agentRisingLow = {
        "................",
        "..........HHHH..",
        "........HHHHHH..",
        "........HBBBBH..",
        ".........HHHH...",
        "......TTTTTT....",
        "....TTTTTT......",
        "...TTTT.........",
        "...LL...........",
        "..LL............",
        "................"
    };

    //Arrodillado, aun de espaldas.
    const std::vector<std::string> agentKneelingBack = {
        "...HHHH.......",
        "..HHHHHH......",
        "..HBBBBH......",
        "..HBBBBH......",
        "...HHHH.......",
        "..TTTTTT......",
        ".TTTTTTTT.....",
        "...TTTT.......",
        "..LL.LL.......",
        ".LL...LL......"
    };

    const std::vector<std::string> agentPocket = {
        "...HHHH.......",
        "..HHHHHH......",
        "..HFFFFH......",
        "..HFEEFH......",
        "..HFFFFH......",
        "...HHHH.......",
        "...TTTT.......",
        "..TTTTT.......",
        "..T.TTTT......",
        "...TTT.T......",
        "...L..L.......",
        "..LL..LL......"
    };

    const std::vector<std::string> agentBombLow = {
        "...HHHH.......",
        "..HHHHHH......",
        "..HFFFFH......",
        "..HFEEFH......",
        "..HFFFFH......",
        "...HHHH.......",
        "...TTTT.......",
        "..TTTTT.......",
        "..T.TTTT.OOQ..",
        "...TTTT..OO...",
        "...L..L.......",
        "..LL..LL......"
    };

    const std::vector<std::string> agentBombMiddle = {
        "............OOQ.",
        "...........TOO..",
        "...HHHH....T....",
        "..HHHHHH...T....",
        "..HFFFFH...T....",
        "..HFEEFH...T....",
        "..HFFFFH..TT....",
        "...HHHH..TT.....",
        "...TTTT.........",
        "..TTTTTT........",
        "...L..L.........",
        "..LL..LL........"
    };

    const std::vector<std::string> agentBombRaised = {
        "............OOQ.",
        "............OO..",
        "............T...",
        "...HHHH.....T...",
        "..HHHHHH....T...",
        "..HFFFFH....T...",
        "..HFEEFH....T...",
        "..HFFFFH....T...",
        "...HHHH.....T...",
        "...TTTT....TT...",
        "..TTTTTT........",
        "...TTTT.........",
        "...L..L.........",
        "..LL..LL........"
    };

    const std::vector<std::string> agentBombRaisedArmed = {
        "............OOR.",
        "............OO..",
        "............T...",
        "...HHHH.....T...",
        "..HHHHHH....T...",
        "..HFFFFH....T...",
        "..HFEEFH....T...",
        "..HFFFFH....T...",
        "...HHHH.....T...",
        "...TTTT....TT...",
        "..TTTTTT........",
        "...TTTT.........",
        "...L..L.........",
        "..LL..LL........"
    };

    const std::vector<std::string> explosionSmall = {
        "......P......",
        "....PXXX....",
        "...PXXXXX...",
        "....PXXX....",
        "......P......"
    };

    const std::vector<std::string> explosionMedium = {
        ".....P...P.....",
        "...PPXXXPP.....",
        "..PXXXXXXXP....",
        ".PXXXXXXXXXP...",
        "..PXXXXXXXP....",
        "...PPXXXPP.....",
        ".....P...P....."
    };

    const std::vector<std::string> explosionLarge = {
        "....R....P....R....",
        "..P...RXXXR...P...",
        ".R.PXXXXXXXXXP.R..",
        "...XXXXXXXXXXXXX...",
        "P.XXXXXXXXXXXXXXX.P",
        "...XXXXXXXXXXXXX...",
        ".R.PXXXXXXXXXP.R..",
        "..P...RXXXR...P...",
        "....R....P....R...."
    };


    ftxui::Color PixelColor(char pixel){
        using namespace ftxui;

        switch (pixel){
            //Casco metalico
            case 'H':
                return Color::RGB(185, 195, 205);

            //Parte posterior/sombra del casco
            case 'B':
                return Color::RGB(90, 100, 115);

            //Cara
            case 'F':
                return Color::RGB(255, 205, 165);

            //Ojos
            case 'E':
                return Color::Black;

            //Traje
            case 'T':
                return Color::BlueLight;

            //Botas/piernas
            case 'L':
                return Color::RGB(80, 90, 105);

            //Cuerpo de bomba
            case 'O':
                return Color::RGB(105, 105, 110);

            //Mecha normal
            case 'Q':
                return Color::YellowLight;

            //Mecha encendida
            case 'R':
                return Color::RedLight;

            //Explosion
            case 'P':
                return Color::YellowLight;

            //Centro de explosion
            case 'X':
                return Color::White;

            //Suelo
            case 'G':
                return Color::RGB(65, 65, 70);

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

    int BottomAlignedTop(
        const std::vector<std::string>& sprite
    ){
        return groundPixelRow -
            static_cast<int>(sprite.size());
    }

    void PutFragment(
        PixelCanvas& canvas,
        int y,
        int x,
        char pixel,
        bool landed
    ){
        x = std::clamp(
            x,
            0,
            canvasWidth - 1
        );

        y = std::clamp(
            y,
            0,
            groundPixelRow - 1
        );

        if (!landed){
            canvas[y][x] = pixel;
            return;
        }

        //Cuando los restos estan en el piso intentamos
        //que no se tapen entre ellos.
        if (canvas[y][x] == '.'){
            canvas[y][x] = pixel;
            return;
        }

        for (int distance = 1; distance <= 8; ++distance){
            const int right =
                x + distance;

            if (
                right < canvasWidth &&
                canvas[y][right] == '.'
            ){
                canvas[y][right] = pixel;
                return;
            }

            const int left =
                x - distance;

            if (
                left >= 0 &&
                canvas[y][left] == '.'
            ){
                canvas[y][left] = pixel;
                return;
            }
        }

        canvas[y][x] = pixel;
    }
}

ftxui::Element DeathAnimation::RenderFallingSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int spriteHeight =
        static_cast<int>(
            agentFallingBack.size()
        );

    const int startTop =
        -spriteHeight;

    const int landingTop =
        BottomAlignedTop(
            agentFallingBack
        );

    const int lastFallFrame =
        fallEndFrame - 1;

    //Caida acelerada.
    const int top =
        startTop +
        (
            (landingTop - startTop) *
            frame *
            frame
        ) /
        (
            lastFallFrame *
            lastFallFrame
        );

    DrawSpriteCentered(
        canvas,
        agentFallingBack,
        top
    );

    return RenderCanvas(canvas);
}

ftxui::Element DeathAnimation::RenderLyingSequence() const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    DrawSpriteCentered(
        canvas,
        agentLyingBack,
        BottomAlignedTop(
            agentLyingBack
        )
    );

    return RenderCanvas(canvas);
}

ftxui::Element DeathAnimation::RenderRiseSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int riseFrame =
        frame - lyingEndFrame;

    if (riseFrame < 5){
        DrawSpriteCentered(
            canvas,
            agentRisingLow,
            BottomAlignedTop(
                agentRisingLow
            )
        );
    }
    else if (riseFrame < 10){
        DrawSpriteCentered(
            canvas,
            agentKneelingBack,
            BottomAlignedTop(
                agentKneelingBack
            )
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            agentBack,
            BottomAlignedTop(
                agentBack
            )
        );
    }

    return RenderCanvas(canvas);
}

ftxui::Element DeathAnimation::RenderTurnSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int turnFrame =
        frame - riseEndFrame;

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


ftxui::Element DeathAnimation::RenderBombSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    if (frame < bombDrawEndFrame){
        const int bombFrame =
            frame - turnEndFrame;

        if (bombFrame < 4){
            DrawSpriteCentered(
                canvas,
                agentFront,
                BottomAlignedTop(
                    agentFront
                )
            );
        }
        else if (bombFrame < 8){
            DrawSpriteCentered(
                canvas,
                agentPocket,
                BottomAlignedTop(
                    agentPocket
                )
            );
        }
        else{
            DrawSpriteCentered(
                canvas,
                agentBombLow,
                BottomAlignedTop(
                    agentBombLow
                )
            );
        }

        return RenderCanvas(canvas);
    }

    if (frame < bombRaiseEndFrame){
        const int raiseFrame =
            frame - bombDrawEndFrame;

        if (raiseFrame < 5){
            DrawSpriteCentered(
                canvas,
                agentBombLow,
                BottomAlignedTop(
                    agentBombLow
                )
            );
        }
        else if (raiseFrame < 9){
            DrawSpriteCentered(
                canvas,
                agentBombMiddle,
                BottomAlignedTop(
                    agentBombMiddle
                )
            );
        }
        else{
            DrawSpriteCentered(
                canvas,
                agentBombRaised,
                BottomAlignedTop(
                    agentBombRaised
                )
            );
        }

        return RenderCanvas(canvas);
    }

    const int inspectFrame =
        frame - bombRaiseEndFrame;

    if ((inspectFrame / 2) % 2 == 0){
        DrawSpriteCentered(
            canvas,
            agentBombRaised,
            BottomAlignedTop(
                agentBombRaised
            )
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            agentBombRaisedArmed,
            BottomAlignedTop(
                agentBombRaisedArmed
            )
        );
    }

    return RenderCanvas(canvas);
}

//=============================================================
// EXPLOSION
//=============================================================

ftxui::Element DeathAnimation::RenderExplosionSequence(int frame) const{
    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    const int explosionFrame =
        frame - bombInspectEndFrame;

    if (explosionFrame < 3){
        DrawSpriteCentered(
            canvas,
            explosionSmall,
            18
        );
    }
    else if (explosionFrame < 6){
        DrawSpriteCentered(
            canvas,
            explosionMedium,
            16
        );
    }
    else{
        DrawSpriteCentered(
            canvas,
            explosionLarge,
            14
        );
    }

    return RenderCanvas(canvas);
}

//=============================================================
// RESTOS DEL AGENTE
//=============================================================

ftxui::Element DeathAnimation::RenderDebrisSequence(int frame) const{
    struct Fragment{
        int startX;
        int startY;

        int velocityX;
        int velocityY;

        char pixel;
    };

    const std::array<Fragment, 38> fragments = {{
        {-3,  0, -8, -5, 'H'},
        {-2, -1, -7, -6, 'H'},
        {-1,  1, -6, -5, 'F'},
        { 0,  0, -5, -7, 'T'},
        { 1, -1, -4, -6, 'H'},
        { 2,  1, -3, -5, 'F'},
        { 3,  0, -2, -7, 'T'},

        {-4,  1, -1, -5, 'L'},
        {-5, -1,  1, -6, 'T'},
        { 4,  0,  2, -5, 'H'},
        { 5,  1,  3, -7, 'T'},
        { 6, -1,  4, -5, 'L'},

        {-1,  2,  5, -6, 'F'},
        { 1,  2,  6, -5, 'T'},
        {-3,  2,  7, -4, 'H'},
        { 3,  2,  8, -6, 'L'},

        {-6,  0, -9, -4, 'T'},
        { 6,  0,  9, -5, 'H'},
        {-2,  0, -5, -3, 'F'},
        { 2,  0,  5, -4, 'T'},

        {-4, -2, -7, -5, 'L'},
        { 4, -2,  7, -6, 'H'},
        { 0, -2, -2, -8, 'T'},
        { 0,  2,  2, -5, 'F'},

        {-7,  1, -8, -3, 'H'},
        { 7,  1,  8, -4, 'L'},
        {-5,  2, -6, -2, 'T'},
        { 5,  2,  6, -3, 'F'},

        {-1, -3, -3, -7, 'H'},
        { 1, -3,  3, -8, 'T'},

        {-8,  0, -10, -3, 'L'},
        { 8,  0,  10, -4, 'H'},

        {-3, -3, -4, -6, 'O'},
        { 3, -3,  4, -7, 'O'},

        {-2,  3, -2, -4, 'T'},
        { 2,  3,  2, -5, 'L'},

        {-1,  0, -1, -6, 'H'},
        { 1,  0,  1, -6, 'F'}
    }};

    PixelCanvas canvas =
        CreateCanvas();

    DrawGround(canvas);

    constexpr int centerX =
        canvasWidth / 2;

    constexpr int startY = 21;

    const int debrisFrame =
        frame - explosionEndFrame;

    const int fallDuration =
        debrisFallEndFrame -
        explosionEndFrame;

    const int currentTime =
        std::clamp(
            debrisFrame,
            0,
            fallDuration
        );

    for (const auto& fragment : fragments){
        int landingTime =
            fallDuration;

        bool foundLanding =
            false;

        for (int testTime = 0; testTime <= fallDuration; ++testTime){
            const int testY =
                startY +
                fragment.startY +
                (
                    fragment.velocityY *
                    testTime
                ) / 4 +
                (
                    testTime *
                    testTime
                ) / 11;

            if (testY >= groundPixelRow - 1){
                landingTime =
                    testTime;

                foundLanding =
                    true;

                break;
            }
        }

        const bool landed =
            currentTime >= landingTime ||
            (
                currentTime >= fallDuration &&
                !foundLanding
            );

        int simulationTime =
            currentTime;

        if (landed){
            simulationTime =
                landingTime;
        }

        int x =
            centerX +
            fragment.startX +
            (
                fragment.velocityX *
                simulationTime
            ) / 4;

        int y =
            startY +
            fragment.startY +
            (
                fragment.velocityY *
                simulationTime
            ) / 4 +
            (
                simulationTime *
                simulationTime
            ) / 11;

        if (landed){
            y =
                groundPixelRow - 1;
        }

        PutFragment(
            canvas,
            y,
            x,
            fragment.pixel,
            landed
        );
    }

    return RenderCanvas(canvas);
}

//=============================================================
// GAME OVER
//=============================================================

ftxui::Element DeathAnimation::RenderGameOver(
    int frame,
    EndReason reason
) const{
    const std::array<std::string, 6> gameOverTitle = {
        " ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ",
        "██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗",
        "██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝",
        "██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗",
        "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║",
        " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝"
    };

    std::string reasonText;

    if (reason == EndReason::noEnergy){
        reasonText =
            "ENERGY DEPLETED";
    }
    else if (reason == EndReason::turnLimit){
        reasonText =
            "TURN LIMIT EXCEEDED";
    }
    else{
        reasonText =
            "SYSTEM FAILURE";
    }

    ftxui::Elements titleLines;

    for (const auto& line : gameOverTitle){
        titleLines.push_back(
            ftxui::text(line) |
            ftxui::color(
                ftxui::Color::RedLight
            ) |
            ftxui::bold |
            ftxui::center
        );
    }

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text(reasonText) |
        ftxui::color(
            ftxui::Color::White
        ) |
        ftxui::bold |
        ftxui::center
    );

    titleLines.push_back(
        ftxui::text("")
    );

    titleLines.push_back(
        ftxui::text(
            "ESCAPE PROTOCOL TERMINATED"
        ) |
        ftxui::color(
            ftxui::Color::Red
        ) |
        ftxui::center
    );

    if (frame >= maxFrames - 1){
        titleLines.push_back(
            ftxui::text("")
        );

        titleLines.push_back(
            ftxui::text("ENTER - PLAY AGAIN    Q - EXIT")
            |ftxui::color(ftxui::Color::GrayLight)
            |ftxui::center
        );
    }

    return UILayout::CenterGameView(
        ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox(
                titleLines
            ) |
            ftxui::size(
                ftxui::WIDTH,
                ftxui::EQUAL,
                canvasWidth
            ),
            ftxui::filler()
        }) |
        ftxui::bgcolor(
            ftxui::Color::Black
        )
    );
}

ftxui::Element DeathAnimation::RenderDeathFrame(
    int frame,
    EndReason reason
) const{
    if (frame < fallEndFrame){
        return RenderFallingSequence(
            frame
        );
    }

    if (frame < lyingEndFrame){
        return RenderLyingSequence();
    }

    if (frame < riseEndFrame){
        return RenderRiseSequence(
            frame
        );
    }

    if (frame < turnEndFrame){
        return RenderTurnSequence(
            frame
        );
    }

    if (frame < bombInspectEndFrame){
        return RenderBombSequence(
            frame
        );
    }

    if (frame < explosionEndFrame){
        return RenderExplosionSequence(
            frame
        );
    }

    if (frame < debrisRestEndFrame){
        return RenderDebrisSequence(
            frame
        );
    }

    return RenderGameOver(
        frame,
        reason
    );
}