#include "circuit_escape/graph.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

void require_graph(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename ExpectedException, typename Function>
void require_graph_throws(Function function, const char* message) {
    bool caught = false;
    try {
        function();
    } catch (const ExpectedException&) {
        caught = true;
    }
    require_graph(caught, message);
}

// Verifica que cada celda tenga exactamente los vecinos permitidos en el orden requerido.
void check_connections(const GraphData& graph) {
    const std::size_t total = graph.rows * graph.columns;
    require_graph(graph.neighbors.size() == total, "Incorrect neighbor list count");

    for (std::size_t id = 0; id < total; ++id) {
        const auto& actual = graph_neighbors(graph, id);
        std::vector<std::size_t> expected;

        if (!graph.blocked[id]) {
            const std::size_t row = id / graph.columns;
            const std::size_t col = id % graph.columns;

            if (row > 0 && !graph.blocked[id - graph.columns])
                expected.push_back(id - graph.columns); // Up
            if (col > 0 && !graph.blocked[id - 1])
                expected.push_back(id - 1);             // Left
            if (col + 1 < graph.columns && !graph.blocked[id + 1])
                expected.push_back(id + 1);             // Right
            if (row + 1 < graph.rows && !graph.blocked[id + graph.columns])
                expected.push_back(id + graph.columns); // Down
        }

        require_graph(actual == expected,
                      "Incorrect neighbors: walls, edges, diagonals, duplicates or order");
        for (std::size_t next : actual) {
            const auto& reverse = graph_neighbors(graph, next);
            require_graph(std::count(reverse.begin(), reverse.end(), id) == 1,
                          "Connection is not bidirectional");
        }
    }
}

void run_graph_tests() {
    // 3 x 3 (nodo 5 es wall)
    const std::vector<bool> blocked = {
        false, false, false,
        false, false, true,
        false, false, false
    };
    const std::vector<int> costs = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    const auto original_blocked = blocked;
    const auto original_costs = costs;

    const GraphData graph = build_graph(3, 3, blocked, costs);
    require_graph(graph.rows == 3 && graph.columns == 3, "Wrong dimensions");
    require_graph(graph.blocked == blocked, "Incorrect blocked cells");
    require_graph(graph.entry_costs == costs, "Incorrect entry costs");
    require_graph(blocked == original_blocked && costs == original_costs,
                  "The original data was changed");
    require_graph(graph_neighbors(graph, 4) == std::vector<std::size_t>{1, 3, 7},
                  "Wrong neighbor order for center cell");
    require_graph(graph_neighbors(graph, 0) == std::vector<std::size_t>{1, 3},
                  "Wrong neighbors for corner cell");
    require_graph(graph_neighbors(graph, 5).empty(), "Wall should not have neighbors");
    check_connections(graph);

    // The Grid adapter must use the same validated adjacency construction.
    Grid<Cell> grid(3, 3);
    grid.at({1, 2}) = Wall{};
    grid.at({0, 2}) = RoughTerrain{4};
    const GraphData adapted = graph_from_grid(grid);
    const std::vector<bool> grid_blocked = {
        false, false, false,
        false, false, true,
        false, false, false
    };
    const std::vector<int> grid_costs = {1, 1, 4, 1, 1, 1, 1, 1, 1};
    const GraphData rebuilt = build_graph(3, 3, grid_blocked, grid_costs);
    require_graph(adapted.neighbors == rebuilt.neighbors &&
                  adapted.blocked == rebuilt.blocked &&
                  adapted.entry_costs == rebuilt.entry_costs,
                  "Grid conversion must match the common graph builder");
    check_connections(adapted);

    // Celda única con mapa cuadrado
    const auto single = build_graph(1, 1, {false}, {0});
    require_graph(graph_neighbors(single, 0).empty(), "Single cell must be isolated");
    check_connections(single);

    const auto rectangle = build_graph(4, 5, std::vector<bool>(20, false),
                                       std::vector<int>(20, 1));
    require_graph(graph_neighbors(rectangle, 7) == std::vector<std::size_t>{2, 6, 8, 12},
                  "Wrong neighbors for a 4x5 map");
    check_connections(rectangle);

    const auto walls = build_graph(2, 2, std::vector<bool>(4, true),
                                   std::vector<int>(4, 1));
    check_connections(walls);

    // Entradas no válidas se rechazan
    require_graph_throws<std::invalid_argument>(
        [] { (void)build_graph(0, 3, {}, {}); }, "Zero rows accepted");
    require_graph_throws<std::invalid_argument>(
        [] { (void)build_graph(3, 0, {}, {}); }, "Zero columns accepted");
    require_graph_throws<std::invalid_argument>(
        [] { (void)build_graph(2, 2, {false}, {1, 1, 1, 1}); },
        "Incorrect blocked vector size accepted");
    require_graph_throws<std::invalid_argument>(
        [] { (void)build_graph(2, 2, std::vector<bool>(4, false), {1, 1}); },
        "Incorrect entry-cost vector size accepted");
    require_graph_throws<std::invalid_argument>(
        [] { (void)build_graph(1, 2, {false, false}, {1, -1}); },
        "Negative entry cost accepted");

    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    require_graph_throws<std::overflow_error>(
        [maximum] { (void)build_graph(maximum, 2, {}, {}); },
        "Dimensions overflow was not detected");
    require_graph_throws<std::out_of_range>(
        [&graph] { (void)graph_neighbors(graph, 9); },
        "Invalid vertex index accepted");

    std::cout << "All graph tests passed!\n";
}
