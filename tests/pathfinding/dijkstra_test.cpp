#include "circuit_escape/map_generation/kruskal.h"
#include "circuit_escape/pathfinding/dijkstra.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

GraphData make_graph(std::size_t rows, std::size_t columns) {
    GraphData graph;
    graph.rows = rows;
    graph.columns = columns;
    graph.neighbors.resize(rows * columns);
    graph.blocked.assign(rows * columns, false);
    graph.entry_costs.assign(rows * columns, 1);

    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const std::size_t current = row * columns + column;
            if (row > 0) graph.neighbors[current].push_back(current - columns);
            if (column > 0) graph.neighbors[current].push_back(current - 1);
            if (column + 1 < columns) graph.neighbors[current].push_back(current + 1);
            if (row + 1 < rows) graph.neighbors[current].push_back(current + columns);
        }
    }

    return graph;
}

template<typename Exception, typename Operation>
void expect_exception(Operation operation) {
    bool threw = false;
    try {
        operation();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void test_cheapest_path() {
    GraphData graph = make_graph(2, 3);
    graph.entry_costs[1] = 15;
    const GraphData original = graph;

    const PathResult result = shortest_energy_path(graph, {0, 0}, {0, 2});
    assert(result.found);
    assert(result.cost == 4);
    assert(result.path == (std::vector<Position>{
        {0, 0}, {1, 0}, {1, 1}, {1, 2}, {0, 2}
    }));
    assert(result.explored == 6);
    assert(graph.neighbors == original.neighbors);
    assert(graph.blocked == original.blocked);
    assert(graph.entry_costs == original.entry_costs);
}

void test_entry_costs_and_zero_costs() {
    GraphData graph = make_graph(1, 2);
    graph.entry_costs = {9, 2};

    const PathResult forward = shortest_energy_path(graph, {0, 0}, {0, 1});
    const PathResult backward = shortest_energy_path(graph, {0, 1}, {0, 0});
    assert(forward.cost == 2);
    assert(backward.cost == 9);

    graph = make_graph(1, 3);
    graph.entry_costs = {30, 0, 0};
    assert(shortest_energy_path(graph, {0, 0}, {0, 2}).cost == 0);
    assert(shortest_energy_path(graph, {0, 2}, {0, 0}).cost == 30);

    const PathResult same = shortest_energy_path(graph, {0, 1}, {0, 1});
    assert(same.found);
    assert(same.cost == 0);
    assert(same.path == (std::vector<Position>{{0, 1}}));
}

void test_all_destinations_and_ties() {
    const GraphData graph = make_graph(2, 2);
    const DijkstraSearch search = run_dijkstra(graph, {0, 0});

    assert(search.explored == 4);
    assert(search.distances[0] == 0);
    assert(search.distances[1] == 1);
    assert(search.distances[2] == 1);
    assert(search.distances[3] == 2);
    assert(!search.predecessors[0].has_value());

    const PathResult result = dijkstra_path(graph, search, {1, 1});
    assert(result.found);
    assert(result.cost == 2);
    assert(result.path == (std::vector<Position>{{0, 0}, {0, 1}, {1, 1}}));
    assert(dijkstra_path(graph, search, {0, 0}).path ==
           (std::vector<Position>{{0, 0}}));
}

void test_unreachable_and_blocked_cells() {
    GraphData graph = make_graph(3, 3);
    for (std::size_t row = 0; row < 3; ++row) {
        graph.blocked[row * 3 + 1] = true;
    }

    const DijkstraSearch search = run_dijkstra(graph, {1, 0});
    const PathResult missing = dijkstra_path(graph, search, {1, 2});
    assert(!missing.found);
    assert(missing.path.empty());
    assert(missing.cost == 0);
    assert(missing.explored == 3);
    assert(!dijkstra_path(graph, search, {0, 1}).found);
    assert(dijkstra_path(graph, search, {0, 0}).cost == 1);

    const DijkstraSearch blocked_source = run_dijkstra(graph, {1, 1});
    assert(blocked_source.explored == 0);
    assert(!dijkstra_path(graph, blocked_source, {1, 1}).found);
    assert(!shortest_energy_path(graph, {1, 1}, {1, 2}).found);
}

void test_invalid_graphs_and_positions() {
    GraphData graph = make_graph(2, 2);
    expect_exception<std::out_of_range>([&] {
        (void)run_dijkstra(graph, {2, 0});
    });
    expect_exception<std::out_of_range>([&] {
        (void)shortest_energy_path(graph, {0, 0}, {0, 2});
    });

    DijkstraSearch search = run_dijkstra(graph, {0, 0});
    search.predecessors.pop_back();
    expect_exception<std::invalid_argument>([&] {
        (void)dijkstra_path(graph, search, {1, 1});
    });

    graph.entry_costs[0] = -1;
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph = make_graph(2, 2);
    graph.neighbors[0].push_back(4);
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph = make_graph(2, 2);
    graph.entry_costs.pop_back();
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph = make_graph(2, 2);
    graph.blocked.pop_back();
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph = make_graph(2, 2);
    graph.neighbors.pop_back();
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph = {};
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });

    graph.rows = std::numeric_limits<std::size_t>::max();
    graph.columns = 2;
    expect_exception<std::invalid_argument>([&] {
        (void)run_dijkstra(graph, {0, 0});
    });
}

void test_large_costs() {
    GraphData graph = make_graph(1, 3);
    graph.entry_costs = {1, std::numeric_limits<int>::max(), 2};
    const DijkstraSearch search = run_dijkstra(graph, {0, 0});
    assert(search.distances[2] ==
           static_cast<std::int64_t>(std::numeric_limits<int>::max()) + 2);
    assert(dijkstra_path(graph, search, {0, 1}).cost == std::numeric_limits<int>::max());
    expect_exception<std::overflow_error>([&] {
        (void)dijkstra_path(graph, search, {0, 2});
    });
}

GraphData graph_from_maze(const MazeGenerationResult& maze) {
    GraphData graph = make_graph(maze.grid.rows(), maze.grid.columns());
    for (std::size_t row = 0; row < graph.rows; ++row) {
        for (std::size_t column = 0; column < graph.columns; ++column) {
            const std::size_t index = row * graph.columns + column;
            graph.blocked[index] = !isTraversable(maze.grid.at({row, column}));
        }
    }
    return graph;
}

std::vector<int> bfs_steps(const GraphData& graph, std::size_t source) {
    std::vector<int> steps(graph.rows * graph.columns, -1);
    std::queue<std::size_t> pending;
    steps[source] = 0;
    pending.push(source);

    while (!pending.empty()) {
        const std::size_t current = pending.front();
        pending.pop();
        for (std::size_t next : graph.neighbors[current]) {
            if (!graph.blocked[next] && steps[next] == -1) {
                steps[next] = steps[current] + 1;
                pending.push(next);
            }
        }
    }
    return steps;
}

void test_generated_mazes() {
    for (const auto& [rows, columns] : std::vector<std::pair<std::size_t, std::size_t>>{
             {10, 15}, {20, 30}, {40, 60}}) {
        const MazeGenerationResult maze = generate_maze(rows, columns, 2026u, 5);
        const GraphData graph = graph_from_maze(maze);
        const std::size_t source = columns + 1;
        const DijkstraSearch search = run_dijkstra(graph, {1, 1});
        const std::vector<int> steps = bfs_steps(graph, source);

        for (std::size_t vertex = 0; vertex < steps.size(); ++vertex) {
            if (steps[vertex] == -1) {
                assert(!search.distances[vertex].has_value());
            } else {
                assert(search.distances[vertex] == steps[vertex]);
            }
        }

        const Position destination{2 * ((rows - 1) / 2) - 1,
                                   2 * ((columns - 1) / 2) - 1};
        const PathResult path = dijkstra_path(graph, search, destination);
        assert(path.found);
        assert(path.cost == steps[destination.row * columns + destination.column]);
        assert(path.path.front() == (Position{1, 1}));
        assert(path.path.back() == destination);
        assert(path.path.size() == static_cast<std::size_t>(path.cost) + 1);
    }
}

}

void run_dijkstra_tests() {
    test_cheapest_path();
    test_entry_costs_and_zero_costs();
    test_all_destinations_and_ties();
    test_unreachable_and_blocked_cells();
    test_invalid_graphs_and_positions();
    test_large_costs();
    test_generated_mazes();
}
