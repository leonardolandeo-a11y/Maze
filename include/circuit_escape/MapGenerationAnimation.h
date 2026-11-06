#pragma once

#include "circuit_escape/map_generation/map_sizes.h"
#include <ftxui/dom/elements.hpp>

// Presentation only: the application owns timing and the asynchronous worker.
class MapGenerationAnimation {
public:
    ftxui::Element RenderFrame(int frame, Project2MapSize map_size) const;
};
