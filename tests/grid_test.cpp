#include <cassert>
#include <stdexcept>
#include "circuit_escape/grid.h"
#include "circuit_escape/cells.h"

bool throwsOutOfRange(Grid<Cell, 3, 4>& grid, Position position) {
    try {
        grid.at(position);
        return false;
    }
    catch (const std::out_of_range&) {
        return true;
    }
};

void run_grid_tests() {
    Grid<Cell, 3, 4> grid;

    // -------------------------
    // Dimensions
    // -------------------------

    assert(grid.rows() == 3);
    assert(grid.columns() == 4);

    // -------------------------
    // contains()
    // -------------------------

    assert(grid.contains({0, 0}));
    assert(grid.contains({2, 3}));

    assert(!grid.contains({3, 0}));
    assert(!grid.contains({0, 4}));

    // -------------------------
    // Valid at()
    // -------------------------

    grid.at({1, 2}) = Wall{};

    assert(std::holds_alternative<Wall>(grid.at({1, 2})));

    // -------------------------
    // Invalid row/column
    // -------------------------

    assert(throwsOutOfRange(grid, {3, 0}));
    assert(throwsOutOfRange(grid, {0, 4}));
}