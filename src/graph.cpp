#include "circuit_escape/graph.h"

#include <limits>
#include <stdexcept>

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

            // Arriba
            if (row > 0 && !blocked[id - columns]) {
                adjacent.push_back(id - columns);
            }

            // Derecha
            if (column < columns - 1 && !blocked[id + 1]) {
                adjacent.push_back(id + 1);
            }

            // Abajo
            if (row < rows - 1 && !blocked[id + columns]) {
                adjacent.push_back(id + columns);
            }

            // Izquierda
            if (column > 0 && !blocked[id - 1]) {
                adjacent.push_back(id - 1);
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
