#pragma once 
#include <ftxui/component/component.hpp>

class StartupAnimation{
private:
    static constexpr int gameViewWidth = 90;
    static constexpr int gameViewHeight = 20;

    static constexpr int bootEndFrame = 38;
    static constexpr int glitchStartFrame = 42;

    static constexpr int logoFormationStartFrame = 64;
    static constexpr int titleRevealFrame = 94;

    static constexpr int subtitleFrame = 110;

    static constexpr int loadingStartFrame = 124;
    static constexpr int loadingCompleteFrame = 156;

    static constexpr int maxFrames = 166;
    ftxui::Element RenderStartupFrame(int frame) const;
    ftxui::Element RenderBlankTransition(int frame) const;
    ftxui::Element RenderGlitchSequence(int frame) const;
    ftxui::Element RenderTitleSequence(int frame) const;
public:
    ftxui::Element RenderStartupFrame(int frame) const;

};