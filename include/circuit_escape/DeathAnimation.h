#pragma once

#include "circuit_escape/environment.h"

#include <ftxui/dom/elements.hpp>

class DeathAnimation{
    static constexpr int gameViewWidth = 90;
    static constexpr int gameViewHeight = 20;

    ftxui::Element RenderSystemFailure(int frame, EndReason reason) const;

public:
    static constexpr int maxFrames = 70;

    ftxui::Element RenderDeathFrame(int frame, EndReason reason) const;
};