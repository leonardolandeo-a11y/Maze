#pragma once

#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

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


MazeGenerationResult generate_maze(
    std::size_t rows,
    std::size_t columns,
    std::uint32_t seed,
    std::size_t extra_opening_count = 0
);
