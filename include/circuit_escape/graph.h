#pragma once

#include "circuit_escape/cells.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/project2_contracts.h"

#include <cstddef>
#include <variant>

// Builds the same four-directional graph for map generation, loading, and pathfinding.
// Entering a cell determines its energy cost; walls have no outgoing connections.
inline GraphData graph_from_grid(const Grid<Cell>& grid) {
    GraphData graph;
    graph.rows = grid.rows();
    graph.columns = grid.columns();
    const std::size_t vertex_count = graph.rows * graph.columns;
    graph.neighbors.resize(vertex_count);
    graph.blocked.resize(vertex_count);
    graph.entry_costs.resize(vertex_count, 1);

    for (std::size_t row = 0; row < graph.rows; ++row) {
        for (std::size_t column = 0; column < graph.columns; ++column) {
            const std::size_t vertex = row * graph.columns + column;
            const Cell& cell = grid.at({row, column});
            graph.blocked[vertex] = !isTraversable(cell);
            if (const auto* rough = std::get_if<RoughTerrain>(&cell)) {
                graph.entry_costs[vertex] = rough->energyCost;
            }
            if (graph.blocked[vertex]) {
                continue;
            }

            const auto connect = [&](std::size_t adjacent_row, std::size_t adjacent_column) {
                if (isTraversable(grid.at({adjacent_row, adjacent_column}))) {
                    graph.neighbors[vertex].push_back(adjacent_row * graph.columns + adjacent_column);
                }
            };
            // Keep existing order for deterministic BFS decisions: up, left, right, down.
            if (row > 0) connect(row - 1, column);
            if (column > 0) connect(row, column - 1);
            if (column + 1 < graph.columns) connect(row, column + 1);
            if (row + 1 < graph.rows) connect(row + 1, column);
        }
    }
    return graph;
}
