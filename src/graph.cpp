#include "circuit_escape/graph.h"

#include <limits>
#include <stdexcept>
#include <variant>

GraphData build_graph(
    std::size_t rows,
    std::size_t columns,
    const std::vector<bool>& blocked,
    const std::vector<int>& entry_costs
) {
    if (rows == 0 || columns == 0) {
        throw std::invalid_argument(
            "Grid dimensions must be positive"
        );
    }

    if (rows > std::numeric_limits<std::size_t>::max() / columns) {
        throw std::overflow_error(
            "Grid dimensions overflow size_t"
        );
    }

    const std::size_t total = rows * columns;

    if (blocked.size() != total || entry_costs.size() != total) {
        throw std::invalid_argument(
            "Grid data sizes do not match dimensions"
        );
    }

    for (int cost : entry_costs) {
        if (cost < 0) {
            throw std::invalid_argument(
                "Entry costs must be non-negative"
            );
        }
    }

    GraphData graph{rows, columns, {}, blocked, entry_costs};
    graph.neighbors.resize(total);

    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const std::size_t id = row * columns + column;

            if (blocked[id]) continue;

            auto& adjacent = graph.neighbors[id];

            // Up
            if (row > 0 && !blocked[id - columns]) {
                adjacent.push_back(id - columns);
            }
            // Left
            if (column > 0 && !blocked[id - 1]) {
                adjacent.push_back(id - 1);
            }
            // Right
            if (column + 1 < columns && !blocked[id + 1]) {
                adjacent.push_back(id + 1);
            }
            // Down
            if (row + 1 < rows && !blocked[id + columns]) {
                adjacent.push_back(id + columns);
            }
        }
    }

    return graph;
}

const std::vector<std::size_t>& graph_neighbors(
    const GraphData& graph,
    std::size_t vertex
) {
    return graph.neighbors.at(vertex);
}

GraphData graph_from_grid(const Grid<Cell>& grid) {
    const std::size_t rows = grid.rows();
    const std::size_t columns = grid.columns();
    if (rows == 0 || columns == 0 ||
        rows > std::numeric_limits<std::size_t>::max() / columns) {
        throw std::invalid_argument("Invalid grid dimensions");
    }

    std::vector<bool> blocked(rows * columns);
    std::vector<int> costs(rows * columns, 1);
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const std::size_t id = row * columns + column;
            const Cell& cell = grid.at({row, column});
            blocked[id] = !isTraversable(cell);
            if (const auto* rough = std::get_if<RoughTerrain>(&cell)) {
                costs[id] = rough->energyCost;
            }
        }
    }
    return build_graph(rows, columns, blocked, costs);
}
