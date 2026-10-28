#include <cassert>
#include <iterator>
#include <limits>
#include <stdexcept>
#include "circuit_escape/grid.h"
#include "circuit_escape/cells.h"

bool throwsOutOfRange(Grid<Cell, 3, 4>& grid, Position position) {
    try {
        grid.at(position);
        return false;
    } catch (const std::out_of_range&) {
        return true;
    }
}

void run_grid_tests() {
    // Original compile-time grid tests.
    Grid<Cell, 3, 4> grid;
    assert(grid.rows() == 3);
    assert(grid.columns() == 4);
    assert(grid.contains({0, 0}));
    assert(grid.contains({2, 3}));
    assert(!grid.contains({3, 0}));
    assert(!grid.contains({0, 4}));

    grid.at({1, 2}) = Wall{};
    assert(std::holds_alternative<Wall>(grid.at({1, 2})));
    assert(throwsOutOfRange(grid, {3, 0}));
    assert(throwsOutOfRange(grid, {0, 4}));

    // New runtime-sized grid tests.
    Grid<Cell> runtime_grid(3, 4);
    assert(runtime_grid.rows() == 3);
    assert(runtime_grid.columns() == 4);
    assert(runtime_grid.contains({0, 0}));
    assert(runtime_grid.contains({2, 3}));
    assert(!runtime_grid.contains({3, 0}));
    assert(!runtime_grid.contains({0, 4}));

    runtime_grid.at({1, 2}) = Wall{};
    assert(std::holds_alternative<Wall>(runtime_grid.at({1, 2})));

    const Grid<Cell>& read_only_grid = runtime_grid;
    assert(std::holds_alternative<Wall>(read_only_grid.at({1, 2})));
    assert(std::distance(runtime_grid.begin(), runtime_grid.end()) == 12);
    assert(std::distance(read_only_grid.cbegin(), read_only_grid.cend()) == 12);

    Grid<Cell> copied_grid = runtime_grid;
    copied_grid.at({1, 2}) = Empty{};
    assert(std::holds_alternative<Wall>(runtime_grid.at({1, 2})));

    for (const Position dimensions : {Position{10, 15}, Position{20, 30}, Position{40, 60}}) {
        Grid<Cell> sized_grid(dimensions.row, dimensions.column);
        assert(sized_grid.rows() == dimensions.row);
        assert(sized_grid.columns() == dimensions.column);
        assert(sized_grid.contains({dimensions.row - 1, dimensions.column - 1}));
        assert(std::distance(sized_grid.cbegin(), sized_grid.cend()) ==
               static_cast<std::ptrdiff_t>(dimensions.row * dimensions.column));
    }

    bool invalid_position = false;
    try {
        runtime_grid.at({3, 0});
    } catch (const std::out_of_range&) {
        invalid_position = true;
    }
    assert(invalid_position);

    bool invalid_dimensions = false;
    try {
        Grid<Cell> invalid_grid(0, 4);
        (void)invalid_grid;
    } catch (const std::invalid_argument&) {
        invalid_dimensions = true;
    }
    assert(invalid_dimensions);

    invalid_dimensions = false;
    try {
        Grid<Cell> invalid_grid(std::numeric_limits<std::size_t>::max(), 2);
        (void)invalid_grid;
    } catch (const std::invalid_argument&) {
        invalid_dimensions = true;
    }
    assert(invalid_dimensions);
}
