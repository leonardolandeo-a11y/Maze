#include "circuit_escape/graph/disjoint_set.h"
#include "circuit_escape/map_generation/kruskal.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <variant>
#include <utility>
#include <vector>

namespace {

bool is_wall(const Grid<Cell>& grid, Position position) {
    return std::holds_alternative<Wall>(grid.at(position));
}

bool same_opening(const WallOpening& first, const WallOpening& second) {
    return first.first_room == second.first_room &&
           first.second_room == second.second_room &&
           first.wall == second.wall && first.weight == second.weight;
}

void check_same_maze(const MazeGenerationResult& first, const MazeGenerationResult& second) {
    assert(first.grid.rows() == second.grid.rows());
    assert(first.grid.columns() == second.grid.columns());
    assert(first.room_count == second.room_count);
    assert(first.seed == second.seed);

    for (std::size_t row = 0; row < first.grid.rows(); ++row) {
        for (std::size_t column = 0; column < first.grid.columns(); ++column) {
            const Position position{row, column};
            assert(is_wall(first.grid, position) == is_wall(second.grid, position));
        }
    }
    const auto check_list = [](const auto& a, const auto& b) {
        assert(a.size() == b.size());
        for (std::size_t index = 0; index < a.size(); ++index) {
            assert(same_opening(a[index], b[index]));
        }
    };
    check_list(first.candidate_openings, second.candidate_openings);
    check_list(first.tree_openings, second.tree_openings);
    check_list(first.extra_openings, second.extra_openings);
}

void check_connected_rooms(const MazeGenerationResult& maze) {
    const std::size_t rows = maze.grid.rows();
    const std::size_t columns = maze.grid.columns();
    std::vector<bool> visited(rows * columns, false);
    std::queue<Position> pending;
    pending.push({1, 1});
    visited[columns + 1] = true;

    const auto try_neighbor = [&](Position position) {
        const std::size_t index = position.row * columns + position.column;
        if (!visited[index] && !is_wall(maze.grid, position)) {
            visited[index] = true;
            pending.push(position);
        }
    };

    while (!pending.empty()) {
        const Position current = pending.front();
        pending.pop();
        if (current.row > 0) try_neighbor({current.row - 1, current.column});
        if (current.row + 1 < rows) try_neighbor({current.row + 1, current.column});
        if (current.column > 0) try_neighbor({current.row, current.column - 1});
        if (current.column + 1 < columns) try_neighbor({current.row, current.column + 1});
    }

    for (std::size_t row = 1; row + 1 < rows; row += 2) {
        for (std::size_t column = 1; column + 1 < columns; column += 2) {
            assert(visited[row * columns + column]);
        }
    }
}

// Independent Prim check: tree cost must be the minimum over all candidate walls.
std::uint64_t minimum_spanning_tree_cost(const MazeGenerationResult& maze) {
    const std::size_t room_columns = (maze.grid.columns() - 1) / 2;
    struct Edge {
        std::size_t to;
        std::uint32_t weight;
    };
    std::vector<std::vector<Edge>> neighbors(maze.room_count);
    for (const WallOpening& opening : maze.candidate_openings) {
        const auto to_index = [room_columns](Position room) {
            return ((room.row - 1) / 2) * room_columns + (room.column - 1) / 2;
        };
        const std::size_t first = to_index(opening.first_room);
        const std::size_t second = to_index(opening.second_room);
        neighbors[first].push_back({second, opening.weight});
        neighbors[second].push_back({first, opening.weight});
    }

    using CostEdge = std::pair<std::uint32_t, std::size_t>;
    std::priority_queue<CostEdge, std::vector<CostEdge>, std::greater<CostEdge>> queue;
    std::vector<bool> visited(maze.room_count, false);
    queue.push({0, 0});
    std::uint64_t cost = 0;
    std::size_t count = 0;
    while (!queue.empty()) {
        const auto [weight, current] = queue.top();
        queue.pop();
        if (visited[current]) continue;
        visited[current] = true;
        cost += weight;
        ++count;
        for (const Edge& edge : neighbors[current]) {
            if (!visited[edge.to]) queue.push({edge.weight, edge.to});
        }
    }
    assert(count == maze.room_count);
    return cost;
}

void test_disjoint_set() {
    DisjointSet sets(5);
    assert(sets.size() == 5);
    assert(sets.component_count() == 5);
    assert(sets.unite(0, 1));
    assert(sets.unite(1, 2));
    assert(!sets.unite(0, 2));
    assert(sets.find(0) == sets.find(2));
    assert(sets.find(0) != sets.find(3));
    assert(sets.component_count() == 3);
    assert(sets.unite(3, 4));
    assert(sets.unite(1, 4));
    assert(sets.component_count() == 1);
    assert(!sets.unite(2, 3));

    bool threw = false;
    try { (void)sets.find(5); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    DisjointSet empty(0);
    assert(empty.component_count() == 0);
    threw = false;
    try { (void)empty.find(0); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

void test_dimensions_and_minimum_cost() {
    for (const auto [rows, columns] : std::vector<std::pair<std::size_t, std::size_t>>{
             {3, 3}, {5, 5}, {10, 15}, {20, 30}, {40, 60}}) {
        const MazeGenerationResult maze = generate_maze(rows, columns, 123456u);
        const std::size_t room_rows = (rows - 1) / 2;
        const std::size_t room_columns = (columns - 1) / 2;
        assert(maze.room_count == room_rows * room_columns);
        assert(maze.tree_openings.size() == maze.room_count - 1);
        assert(maze.extra_openings.empty());
        assert(maze.candidate_openings.size() ==
               room_rows * (room_columns - 1) + (room_rows - 1) * room_columns);

        std::uint64_t tree_cost = 0;
        for (const WallOpening& opening : maze.tree_openings) {
            assert(opening.weight >= 1 && opening.weight <= 1'000'000u);
            assert(!is_wall(maze.grid, opening.wall));
            tree_cost += opening.weight;
        }
        assert(tree_cost == minimum_spanning_tree_cost(maze));
        check_connected_rooms(maze);

        for (std::size_t column = 0; column < columns; ++column) {
            assert(is_wall(maze.grid, {0, column}));
            assert(is_wall(maze.grid, {rows - 1, column}));
        }
        for (std::size_t row = 0; row < rows; ++row) {
            assert(is_wall(maze.grid, {row, 0}));
            assert(is_wall(maze.grid, {row, columns - 1}));
        }
    }
}

void test_known_minimum_spanning_tree() {
    // Known 5x5 maze: four rooms and four candidate wall openings.
    // With seed 42, Kruskal must choose the three cheapest connections.
    const MazeGenerationResult maze = generate_maze(5, 5, 42u);
    assert(maze.room_count == 4);
    assert(maze.candidate_openings.size() == 4);
    assert(maze.tree_openings.size() == 3);
    assert(maze.extra_openings.empty());

    const std::array<std::uint32_t, 3> expected_weights{126'068u, 286'877u, 637'543u};
    const std::array<Position, 3> expected_walls{{{2, 1}, {2, 3}, {1, 2}}};
    std::uint64_t actual_total = 0;
    for (std::size_t index = 0; index < expected_weights.size(); ++index) {
        const WallOpening& opening = maze.tree_openings[index];
        assert(opening.weight == expected_weights[index]);
        assert(opening.wall == expected_walls[index]);
        assert(!is_wall(maze.grid, opening.wall));
        actual_total += opening.weight;
    }

    assert(actual_total == 1'050'488ULL);
    assert(is_wall(maze.grid, {3, 2}));
}
void test_seed_and_extra_paths() {
    const MazeGenerationResult base = generate_maze(20, 30, 2026u);
    const MazeGenerationResult same = generate_maze(20, 30, 2026u);
    check_same_maze(base, same);

    const MazeGenerationResult with_extras = generate_maze(20, 30, 2026u, 8);
    const MazeGenerationResult same_extras = generate_maze(20, 30, 2026u, 8);
    check_same_maze(with_extras, same_extras);
    assert(with_extras.tree_openings.size() == base.tree_openings.size());
    assert(with_extras.extra_openings.size() == 8);
    for (std::size_t index = 0; index < base.tree_openings.size(); ++index) {
        assert(same_opening(base.tree_openings[index], with_extras.tree_openings[index]));
    }
    for (const WallOpening& extra : with_extras.extra_openings) {
        assert(!is_wall(with_extras.grid, extra.wall));
        for (const WallOpening& tree : with_extras.tree_openings) {
            assert(!(extra.wall == tree.wall));
        }
    }
    check_connected_rooms(with_extras);

    const MazeGenerationResult all_extras = generate_maze(10, 15, 2026u,
                                                          std::numeric_limits<std::size_t>::max());
    assert(all_extras.extra_openings.size() ==
           all_extras.candidate_openings.size() - all_extras.tree_openings.size());
    check_connected_rooms(all_extras);

    const MazeGenerationResult other_seed = generate_maze(20, 30, 2027u);
    bool different = false;
    for (std::size_t index = 0; index < base.candidate_openings.size(); ++index) {
        if (!same_opening(base.candidate_openings[index], other_seed.candidate_openings[index])) {
            different = true;
            break;
        }
    }
    assert(different);
}

void test_invalid_dimensions() {
    for (const auto [rows, columns] : std::vector<std::pair<std::size_t, std::size_t>>{
             {0, 0}, {2, 10}, {10, 2}, {1, 1}}) {
        bool threw = false;
        try {
            (void)generate_maze(rows, columns, 42u);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }
}

} // namespace

void run_kruskal_tests() {
    test_disjoint_set();
    test_dimensions_and_minimum_cost();
    test_known_minimum_spanning_tree();
    test_seed_and_extra_paths();
    test_invalid_dimensions();
}
