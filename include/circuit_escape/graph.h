#pragma once

#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/project2_contracts.h"

#include <cstddef>
#include <vector>

// Builds an adjacency-list graph; neighbors are in deterministic
// up, left, right, down order, matching existing guard behavior.
GraphData build_graph(
    std::size_t rows,
    std::size_t columns,
    const std::vector<bool>& blocked,
    const std::vector<int>& entry_costs
);

// Convert the existing Grid<Cell> without duplicating graph construction.
GraphData graph_from_grid(const Grid<Cell>& grid);

// Throws std::out_of_range for an invalid vertex.
const std::vector<std::size_t>& graph_neighbors(
    const GraphData& graph,
    std::size_t vertex
);
