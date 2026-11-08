#pragma once

#include "circuit_escape/map_generation/map_sizes.h"
#include "circuit_escape/ui_theme.h"

// Presentation only: the application owns timing and the asynchronous worker.
class MapGenerationAnimation {
public:
    ftxui::Element RenderFrame(int frame, Project2MapSize map_size,
        std::optional<ftxui::Dimensions> terminal = std::nullopt) const;
};
