#include "circuit_escape/map_generation/kruskal.h"

#include "circuit_escape/map_generation/disjoint_set.h"

#include <algorithm>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

MazeGenerationResult::MazeGenerationResult(
    std::size_t rows, std::size_t columns, std::uint32_t seed_value
) : grid(rows, columns), seed(seed_value) {}

MazeGenerationResult generate_maze(
    std::size_t rows,
    std::size_t columns,
    std::uint32_t seed,
    std::size_t extra_opening_count
) {
    if (rows < 3 || columns < 3) {
        throw std::invalid_argument("Maze dimensions must be at least 3x3");
    }

    const std::size_t room_rows = (rows - 1) / 2;
    const std::size_t room_columns = (columns - 1) / 2;
    if (room_rows > std::numeric_limits<std::size_t>::max() / room_columns) {
        throw std::invalid_argument("Maze room count is too large");
    }

    MazeGenerationResult result(rows, columns, seed);
    result.room_count = room_rows * room_columns;

    for (Cell& cell : result.grid) {
        cell = Wall{};
    }

    for (std::size_t row = 0; row < room_rows; ++row) {
        for (std::size_t column = 0; column < room_columns; ++column) {
            result.grid.at({2 * row + 1, 2 * column + 1}) = Empty{};
        }
    }

    std::mt19937 random_engine(seed);

    for (std::size_t row = 0; row < room_rows; ++row) {
        for (std::size_t column = 0; column < room_columns; ++column) {
            const Position first{2 * row + 1, 2 * column + 1};
            if (column + 1 < room_columns) {
                result.candidate_openings.push_back({
                    first, {first.row, first.column + 2},
                    {first.row, first.column + 1}, static_cast<std::uint32_t>(1u + (random_engine() % 1'000'000u))
                });
            }
            if (row + 1 < room_rows) {
                result.candidate_openings.push_back({
                    first, {first.row + 2, first.column},
                    {first.row + 1, first.column}, static_cast<std::uint32_t>(1u + (random_engine() % 1'000'000u))
                });
            }
        }
    }

    std::sort(result.candidate_openings.begin(), result.candidate_openings.end(),
              [](const WallOpening& first, const WallOpening& second) {
                  if (first.weight != second.weight) {
                      return first.weight < second.weight;
                  }
                  if (first.wall.row != second.wall.row) {
                      return first.wall.row < second.wall.row;
                  }
                  return first.wall.column < second.wall.column;
              });

    const auto room_id = [room_columns](Position position) {
        return ((position.row - 1) / 2) * room_columns + (position.column - 1) / 2;
    };

    DisjointSet components(result.room_count);
    std::vector<WallOpening> remaining;

    for (const WallOpening& opening : result.candidate_openings) {
        if (components.unite(room_id(opening.first_room), room_id(opening.second_room))) {
            result.tree_openings.push_back(opening);
            result.grid.at(opening.wall) = Empty{};
        } else {
            remaining.push_back(opening);
        }
    }

    if (components.component_count() != 1) {
        throw std::logic_error("Kruskal failed to connect all rooms");
    }
    for (std::size_t count = remaining.size(); count > 1; --count) {
        const std::size_t index = random_engine() % count;
        std::swap(remaining[count - 1], remaining[index]);
    }

    const std::size_t number_of_extras = std::min(extra_opening_count, remaining.size());
    for (std::size_t index = 0; index < number_of_extras; ++index) {
        result.extra_openings.push_back(remaining[index]);
        result.grid.at(remaining[index].wall) = Empty{};
    }

    return result;
}
