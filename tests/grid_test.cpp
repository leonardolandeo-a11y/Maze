#include <cassert>
#include <stdexcept>
#include <limits>
#include <iterator>
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

    Grid<Cell> dynamicGrid(10, 15);
    assert(dynamicGrid.rows() == 10);
    assert(dynamicGrid.columns() == 15);
    assert(dynamicGrid.contains({9, 14}));
    assert(!dynamicGrid.contains({10, 0}));
    assert(!dynamicGrid.contains({0, 15}));

    dynamicGrid.at({7, 4}) = Wall{};
    const auto& constGrid = dynamicGrid;
    assert(std::holds_alternative<Wall>(constGrid.at({7, 4})));
    assert(std::distance(constGrid.begin(), constGrid.end()) == 150);

    bool invalidPosition = false;
    try {
        dynamicGrid.at({10, 0});
    } catch (const std::out_of_range&) {
        invalidPosition = true;
    }
    assert(invalidPosition);

    bool invalidDimensions = false;
    try {
        Grid<Cell> emptyGrid(0, 15);
    } catch (const std::invalid_argument&) {
        invalidDimensions = true;
    }
    assert(invalidDimensions);

    bool overflowRejected = false;
    try {
        Grid<Cell> hugeGrid(std::numeric_limits<std::size_t>::max(), 2);
    } catch (const std::invalid_argument&) {
        overflowRejected = true;
    }
    assert(overflowRejected);
}