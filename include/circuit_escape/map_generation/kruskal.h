#pragma once

#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

// One candidate wall between two room centers, with an independent Kruskal weight.
// The weight orders wall removals; it is NOT an energy cost for the player.
struct WallOpening {
    Position first_room;
    Position second_room;
    Position wall;
    std::uint32_t weight{};
};

struct MazeGenerationResult {
    Grid<Cell> grid;
    std::vector<WallOpening> candidate_openings;
    std::vector<WallOpening> tree_openings;
    std::vector<WallOpening> extra_openings;
    std::uint32_t seed{};
    std::size_t room_count{};

    MazeGenerationResult(std::size_t rows, std::size_t columns, std::uint32_t seed_value);
};

// Builds maze geometry only. Objective and guard placement is a later step.
// A one-cell wall border is preserved. Room centers use odd/odd coordinates.
// extra_opening_count is capped at the number of non-tree candidate walls.
MazeGenerationResult generate_maze(
    std::size_t rows,
    std::size_t columns,
    std::uint32_t seed,
    std::size_t extra_opening_count = 0
);
